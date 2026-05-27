#pragma once
/**
 * relation_engine.hpp
 * Math Relation Analyser — C++ Core Engine
 *
 * Self-contained header-only core.
 * Compiled either as:
 *   (a) native CLI binary  →  g++ -std=c++17 main.cpp -o math-analyser
 *   (b) WASM via Emscripten → see build.sh
 */

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>
#include <functional>
#include <stdexcept>

namespace math {

constexpr int MAX_ELEMENTS = 12;

// ── Types ─────────────────────────────────────────────────────

using Element = std::string;
using Pair    = std::pair<Element, Element>;
using Set     = std::vector<Element>;
using PairVec = std::vector<Pair>;

// ── Result structs ─────────────────────────────────────────────

struct Properties {
    bool reflexive     = false;
    bool irreflexive   = false;
    bool symmetric     = false;
    bool antisymmetric = false;
    bool asymmetric    = false;
    bool transitive    = false;
    bool total         = false;

    std::vector<std::string> refl_violations;
    std::vector<std::string> irrefl_violations;
    std::vector<std::string> sym_violations;
    std::vector<std::string> antisym_violations;
    std::vector<std::string> trans_violations;
};

struct Classification {
    std::string label;
    std::string css_class;  // for JS bridge
};

struct Inference {
    std::string title;
    std::string body;
};

struct SpecialRelations {
    bool empty     = false;
    bool universal = false;
    bool identity  = false;
};

/** Function / mapping properties (relation as map from A to A). */
struct MappingInfo {
    bool is_function   = false;
    bool is_partial_fn = false;
    bool injective     = false;
    bool surjective    = false;
    bool bijective     = false;
    std::string summary;
};

/** Computed relation operations on A (closures, composition, inverse). */
struct RelationOps {
    PairVec inverse;
    PairVec complement;
    PairVec compose_rr;           // R ∘ R
    PairVec reflexive_closure;
    PairVec symmetric_closure;
    PairVec transitive_closure;
    bool    rr_subset_r = false;  // R² ⊆ R (transitivity check via composition)
};

struct HasseInfo {
    bool is_partial_order = false;
    PairVec covers;
    std::vector<Set> levels;
    Set minimal;
    Set maximal;
    Element least;
    Element greatest;
    PairVec incomparable;
    std::string explanation;
};

struct AnalysisResult {
    Set              elements;
    PairVec          pairs;
    int              n = 0;

    // Boolean adjacency matrix [i][j]
    std::vector<std::vector<int>> matrix;

    Properties                  props;
    SpecialRelations            special;
    MappingInfo                 mapping;
    RelationOps                 operations;
    HasseInfo                   hasse;
    std::vector<Classification> classifications;
    std::vector<Inference>      inferences;
    bool                        vacuous = false;
};

// ── Parse helpers ──────────────────────────────────────────────

inline Set parse_set(const std::string& raw) {
    Set elements;
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ',')) {
        size_t start = token.find_first_not_of(" \t\r\n");
        size_t end   = token.find_last_not_of(" \t\r\n");
        if (start != std::string::npos)
            elements.push_back(token.substr(start, end - start + 1));
    }
    return elements;
}

inline PairVec pairs_from_set(const std::set<Pair>& s) {
    return PairVec(s.begin(), s.end());
}

inline std::set<Pair> pair_set_from_vec(const PairVec& pairs) {
    return std::set<Pair>(pairs.begin(), pairs.end());
}

inline PairVec compute_inverse(const PairVec& pairs) {
    PairVec out;
    out.reserve(pairs.size());
    for (const auto& [a, b] : pairs) out.push_back({b, a});
    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

inline PairVec compute_complement(const Set& elements, const PairVec& pairs) {
    std::set<Pair> in_r(pairs.begin(), pairs.end());
    PairVec out;
    for (const auto& a : elements)
        for (const auto& b : elements)
            if (in_r.count({a, b}) == 0) out.push_back({a, b});
    return out;
}

inline PairVec compute_compose(const PairVec& r, const PairVec& s) {
    std::set<Pair> out;
    for (const auto& [a, b] : r)
        for (const auto& [c, d] : s)
            if (b == c) out.insert({a, d});
    return pairs_from_set(out);
}

inline PairVec compute_reflexive_closure(const Set& elements, const PairVec& pairs) {
    std::set<Pair> out(pairs.begin(), pairs.end());
    for (const auto& a : elements) out.insert({a, a});
    return pairs_from_set(out);
}

inline PairVec compute_symmetric_closure(const PairVec& pairs) {
    std::set<Pair> out(pairs.begin(), pairs.end());
    for (const auto& [a, b] : pairs) out.insert({b, a});
    return pairs_from_set(out);
}

inline PairVec compute_transitive_closure(const Set& elements, const PairVec& pairs) {
    std::map<Element, int> idx;
    for (int i = 0; i < static_cast<int>(elements.size()); ++i) idx[elements[i]] = i;
    const int n = static_cast<int>(elements.size());
    std::vector<std::vector<bool>> reach(n, std::vector<bool>(n, false));
    for (const auto& [a, b] : pairs) {
        if (idx.count(a) && idx.count(b)) reach[idx[a]][idx[b]] = true;
    }
    for (int k = 0; k < n; ++k)
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                if (reach[i][k] && reach[k][j]) reach[i][j] = true;

    PairVec out;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (reach[i][j]) out.push_back({elements[i], elements[j]});
    return out;
}

inline bool is_subset(const std::set<Pair>& sub, const std::set<Pair>& sup) {
    for (const auto& p : sub)
        if (sup.count(p) == 0) return false;
    return true;
}

inline Set parse_set_validated(const std::string& raw) {
    Set elements = parse_set(raw);
    if (elements.empty())
        throw std::invalid_argument("Set A is empty. Enter at least one element.");
    if (static_cast<int>(elements.size()) > MAX_ELEMENTS)
        throw std::invalid_argument(
            "Maximum " + std::to_string(MAX_ELEMENTS) + " elements supported.");

    std::set<std::string> seen;
    for (const auto& e : elements) {
        if (!seen.insert(e).second)
            throw std::invalid_argument("Set A has duplicate elements: '" + e + "'");
    }
    return elements;
}

/**
 * parse_pairs
 * Accepts: "(1,2); (2,3)" or "(1,2)\n(2,3)" or "1,2;2,3"
 * Throws std::invalid_argument on bad input or elements not in set.
 */
inline PairVec parse_pairs(const std::string& raw, const Set& elements) {
    PairVec pairs;
    if (raw.empty()) return pairs;

    std::set<std::string> elem_set(elements.begin(), elements.end());
    std::set<Pair> seen;

    auto push_pair = [&](const std::string& a, const std::string& b) {
        auto clean_token = [](std::string s) {
            size_t start = s.find_first_not_of(" \t\r\n");
            size_t end = s.find_last_not_of(" \t\r\n");
            return start == std::string::npos ? std::string() : s.substr(start, end - start + 1);
        };
        std::string left = clean_token(a);
        std::string right = clean_token(b);
        if (elem_set.find(left) == elem_set.end())
            throw std::invalid_argument("Element '" + left + "' not in set A");
        if (elem_set.find(right) == elem_set.end())
            throw std::invalid_argument("Element '" + right + "' not in set A");
        Pair p{left, right};
        if (seen.insert(p).second) pairs.push_back(p);
    };

    bool saw_parenthesised = false;
    std::string leftovers = raw;
    size_t pos = 0;
    while (pos < raw.size()) {
        size_t open = raw.find('(', pos);
        if (open == std::string::npos) break;
        size_t close = raw.find(')', open + 1);
        if (close == std::string::npos)
            throw std::invalid_argument("Unclosed ordered pair in relation R.");

        std::string inner = raw.substr(open + 1, close - open - 1);
        size_t comma = inner.find(',');
        if (comma == std::string::npos)
            throw std::invalid_argument("Cannot parse ordered pair: (" + inner + ")");
        push_pair(inner.substr(0, comma), inner.substr(comma + 1));
        saw_parenthesised = true;

        for (size_t i = open; i <= close; ++i) leftovers[i] = ' ';
        pos = close + 1;
    }

    if (saw_parenthesised) {
        for (char c : leftovers) {
            if (c != ' ' && c != '\t' && c != '\r' && c != '\n' && c != ';' && c != ',')
                throw std::invalid_argument("Cannot parse relation R near: " + leftovers);
        }
        return pairs;
    }

    std::string normalised = raw;
    for (char& c : normalised)
        if (c == '\n' || c == '\r') c = ';';

    std::stringstream ss(normalised);
    std::string token;
    while (std::getline(ss, token, ';')) {
        if (token.find_first_not_of(" \t\r\n") == std::string::npos) continue;
        size_t comma = token.find(',');
        if (comma == std::string::npos)
            throw std::invalid_argument("Cannot parse token: " + token);
        push_pair(token.substr(0, comma), token.substr(comma + 1));
    }

    return pairs;
}

// ── Core engine ────────────────────────────────────────────────

class RelationEngine {
public:

    static AnalysisResult analyse(const Set& elements, const PairVec& pairs) {
        AnalysisResult res;
        res.elements = elements;
        res.pairs    = pairs;
        res.n        = static_cast<int>(elements.size());

        // Index map: element → index
        std::map<Element, int> idx;
        for (int i = 0; i < res.n; ++i) idx[elements[i]] = i;

        // Build adjacency set and matrix
        std::set<Pair> pair_set(pairs.begin(), pairs.end());
        auto has = [&](const Element& a, const Element& b) {
            return pair_set.count({a, b}) > 0;
        };

        res.matrix.assign(res.n, std::vector<int>(res.n, 0));
        for (auto& [a, b] : pairs) {
            res.matrix[idx[a]][idx[b]] = 1;
        }

        // ── Properties ──────────────────────────────────────
        auto& p = res.props;
        p.reflexive     = true;
        p.irreflexive   = true;
        p.symmetric     = true;
        p.antisymmetric = true;
        p.transitive    = true;
        p.total         = true;

        // Reflexive / Irreflexive
        for (const auto& a : elements) {
            if (!has(a, a)) {
                p.reflexive = false;
                if (p.refl_violations.size() < 3)
                    p.refl_violations.push_back("(" + a + "," + a + ")");
            } else {
                p.irreflexive = false;
                if (p.irrefl_violations.size() < 3)
                    p.irrefl_violations.push_back("(" + a + "," + a + ")");
            }
        }

        // Symmetric / Antisymmetric
        for (const auto& a : elements) {
            for (const auto& b : elements) {
                if (a == b) continue;
                if (has(a, b)) {
                    if (!has(b, a)) {
                        p.symmetric = false;
                        if (p.sym_violations.size() < 3)
                            p.sym_violations.push_back(
                                "(" + a + "," + b + ")∈R but (" + b + "," + a + ")∉R");
                    }
                    if (has(b, a)) {
                        p.antisymmetric = false;
                        if (p.antisym_violations.size() < 2)
                            p.antisym_violations.push_back(
                                "(" + a + "," + b + ") & (" + b + "," + a + ")");
                    }
                }
            }
        }

        // Asymmetric = irreflexive + antisymmetric
        p.asymmetric = p.irreflexive && p.antisymmetric;

        // Transitive
        for (const auto& a : elements) {
            for (const auto& b : elements) {
                for (const auto& c : elements) {
                    if (has(a, b) && has(b, c) && !has(a, c)) {
                        p.transitive = false;
                        if (p.trans_violations.size() < 3)
                            p.trans_violations.push_back(
                                "(" + a + "," + b + "),(" + b + "," + c +
                                ") ⟹ need (" + a + "," + c + ")");
                    }
                }
            }
        }

        // Total (connex)
        for (const auto& a : elements) {
            for (const auto& b : elements) {
                if (!has(a, b) && !has(b, a)) { p.total = false; goto done_total; }
            }
        }
        done_total:;

        // ── Special relations ────────────────────────────────
        auto& sp = res.special;
        sp.empty     = pairs.empty();
        sp.universal = (static_cast<int>(pair_set.size()) == res.n * res.n);
        sp.identity  = (static_cast<int>(pair_set.size()) == res.n)
                    && std::all_of(elements.begin(), elements.end(),
                                   [&](const Element& a){ return has(a,a); })
                    && std::all_of(pairs.begin(), pairs.end(),
                                   [](const Pair& pr){ return pr.first == pr.second; });

        res.vacuous = sp.empty;

        // ── Mapping (function) analysis on A → A ─────────────
        _analyse_mapping(res, elements, has);

        // ── Relation operations ──────────────────────────────
        _compute_operations(res, elements, pairs, pair_set);

        // ── Hasse diagram data (for partial orders) ──────────
        _compute_hasse(res, elements, has);

        // ── Classification ───────────────────────────────────
        auto add = [&](const std::string& label, const std::string& cls) {
            res.classifications.push_back({label, cls});
        };

        if (sp.empty)     add("Empty relation",          "badge-empty");
        if (sp.universal) add("Universal relation",      "badge-univ");
        if (sp.identity)  add("Identity relation",       "badge-sp");

        if (p.reflexive && p.symmetric && p.transitive)
            add("Equivalence relation", "badge-eq");

        if (p.reflexive && p.antisymmetric && p.transitive && p.total)
            add("Total order (chain)",   "badge-po");
        else if (p.reflexive && p.antisymmetric && p.transitive)
            add("Partial order (poset)", "badge-po");

        if (p.irreflexive && p.antisymmetric && p.transitive && p.total)
            add("Strict total order",    "badge-po");
        else if (p.irreflexive && p.antisymmetric && p.transitive)
            add("Strict partial order",  "badge-sp");

        if (p.reflexive && p.transitive && !p.symmetric && !p.antisymmetric)
            add("Preorder",              "badge-sp");

        if (p.reflexive && p.symmetric && !p.transitive)
            add("Tolerance relation",    "badge-none");

        if (res.mapping.bijective)
            add("Bijection",               "badge-fn");
        else if (res.mapping.is_function)
            add("Function (mapping)",      "badge-fn");
        else if (res.mapping.is_partial_fn && !pairs.empty())
            add("Partial function",      "badge-sp");

        if (res.classifications.empty())
            add("General relation",      "badge-none");

        // ── Inferences ───────────────────────────────────────
        _build_inferences(res, elements, has);

        return res;
    }

    // ── JSON serialiser (for WASM ↔ JS bridge) ──────────────

    static std::string to_json(const AnalysisResult& r) {
        std::ostringstream o;
        auto esc = [](const std::string& s) -> std::string {
            std::string out;
            for (char c : s) {
                if (c == '"')  out += "\\\"";
                else if (c == '\\') out += "\\\\";
                else if (c == '\n') out += "\\n";
                else out += c;
            }
            return out;
        };

        o << "{\n";

        // elements
        o << "  \"elements\": [";
        for (int i = 0; i < r.n; ++i) {
            if (i) o << ", ";
            o << "\"" << esc(r.elements[i]) << "\"";
        }
        o << "],\n";

        // pairs
        o << "  \"pairs\": [";
        for (size_t i = 0; i < r.pairs.size(); ++i) {
            if (i) o << ", ";
            o << "[\"" << esc(r.pairs[i].first) << "\",\"" << esc(r.pairs[i].second) << "\"]";
        }
        o << "],\n";

        // matrix
        o << "  \"matrix\": [";
        for (int i = 0; i < r.n; ++i) {
            if (i) o << ", ";
            o << "[";
            for (int j = 0; j < r.n; ++j) {
                if (j) o << ",";
                o << r.matrix[i][j];
            }
            o << "]";
        }
        o << "],\n";

        // props
        auto& p = r.props;
        o << "  \"props\": {\n";
        o << "    \"refl\":"     << (p.reflexive     ? "true" : "false") << ",\n";
        o << "    \"irrefl\":"   << (p.irreflexive   ? "true" : "false") << ",\n";
        o << "    \"sym\":"      << (p.symmetric     ? "true" : "false") << ",\n";
        o << "    \"antisym\":"  << (p.antisymmetric ? "true" : "false") << ",\n";
        o << "    \"asym\":"     << (p.asymmetric    ? "true" : "false") << ",\n";
        o << "    \"trans\":"    << (p.transitive    ? "true" : "false") << ",\n";
        o << "    \"total\":"    << (p.total         ? "true" : "false") << ",\n";

        auto arr = [&](const std::string& key,
                       const std::vector<std::string>& v) {
            o << "    \"" << key << "\": [";
            for (size_t i = 0; i < v.size(); ++i) {
                if (i) o << ", ";
                o << "\"" << esc(v[i]) << "\"";
            }
            o << "]";
        };
        arr("reflViol",    p.refl_violations);    o << ",\n";
        arr("irreflViol",  p.irrefl_violations);  o << ",\n";
        arr("symViol",     p.sym_violations);     o << ",\n";
        arr("antisymViol", p.antisym_violations); o << ",\n";
        arr("transViol",   p.trans_violations);   o << "\n";
        o << "  },\n";

        // mapping
        auto& m = r.mapping;
        o << "  \"mapping\": {";
        o << "\"isFunction\":"   << (m.is_function   ? "true" : "false") << ",";
        o << "\"isPartialFn\":" << (m.is_partial_fn ? "true" : "false") << ",";
        o << "\"injective\":"    << (m.injective     ? "true" : "false") << ",";
        o << "\"surjective\":"   << (m.surjective    ? "true" : "false") << ",";
        o << "\"bijective\":"    << (m.bijective     ? "true" : "false") << ",";
        o << "\"summary\":\"" << esc(m.summary) << "\"},\n";

        // special
        auto& sp = r.special;
        o << "  \"special\": {";
        o << "\"empty\":"     << (sp.empty     ? "true" : "false") << ",";
        o << "\"universal\":" << (sp.universal ? "true" : "false") << ",";
        o << "\"identity\":"  << (sp.identity  ? "true" : "false") << "},\n";

        // operations
        auto pair_arr = [&](const PairVec& pv) {
            o << "[";
            for (size_t i = 0; i < pv.size(); ++i) {
                if (i) o << ", ";
                o << "[\"" << esc(pv[i].first) << "\",\"" << esc(pv[i].second) << "\"]";
            }
            o << "]";
        };
        auto& op = r.operations;
        o << "  \"operations\": {\n";
        o << "    \"inverse\": ";             pair_arr(op.inverse);             o << ",\n";
        o << "    \"complement\": ";          pair_arr(op.complement);          o << ",\n";
        o << "    \"composeRR\": ";           pair_arr(op.compose_rr);          o << ",\n";
        o << "    \"reflexiveClosure\": ";    pair_arr(op.reflexive_closure);   o << ",\n";
        o << "    \"symmetricClosure\": ";    pair_arr(op.symmetric_closure);   o << ",\n";
        o << "    \"transitiveClosure\": ";   pair_arr(op.transitive_closure);  o << ",\n";
        o << "    \"rrSubsetR\": " << (op.rr_subset_r ? "true" : "false") << "\n";
        o << "  },\n";

        // hasse
        auto set_arr = [&](const Set& set) {
            o << "[";
            for (size_t i = 0; i < set.size(); ++i) {
                if (i) o << ", ";
                o << "\"" << esc(set[i]) << "\"";
            }
            o << "]";
        };
        auto& h = r.hasse;
        o << "  \"hasse\": {\n";
        o << "    \"isPartialOrder\": " << (h.is_partial_order ? "true" : "false") << ",\n";
        o << "    \"covers\": "; pair_arr(h.covers); o << ",\n";
        o << "    \"levels\": [";
        for (size_t i = 0; i < h.levels.size(); ++i) {
            if (i) o << ", ";
            set_arr(h.levels[i]);
        }
        o << "],\n";
        o << "    \"minimal\": "; set_arr(h.minimal); o << ",\n";
        o << "    \"maximal\": "; set_arr(h.maximal); o << ",\n";
        o << "    \"least\": \"" << esc(h.least) << "\",\n";
        o << "    \"greatest\": \"" << esc(h.greatest) << "\",\n";
        o << "    \"incomparable\": "; pair_arr(h.incomparable); o << ",\n";
        o << "    \"explanation\": \"" << esc(h.explanation) << "\"\n";
        o << "  },\n";

        // classifications
        o << "  \"classifications\": [";
        for (size_t i = 0; i < r.classifications.size(); ++i) {
            if (i) o << ", ";
            o << "{\"label\":\"" << esc(r.classifications[i].label)
              << "\",\"cls\":\"" << esc(r.classifications[i].css_class) << "\"}";
        }
        o << "],\n";

        // inferences
        o << "  \"inferences\": [";
        for (size_t i = 0; i < r.inferences.size(); ++i) {
            if (i) o << ", ";
            o << "{\"title\":\"" << esc(r.inferences[i].title)
              << "\",\"body\":\"" << esc(r.inferences[i].body) << "\"}";
        }
        o << "]\n";

        o << "}";
        return o.str();
    }

private:

    static void _analyse_mapping(
        AnalysisResult& res,
        const Set& elements,
        std::function<bool(const Element&, const Element&)> has)
    {
        const int n = res.n;
        if (n == 0) return;

        std::map<Element, int> out_deg, in_deg;
        for (const auto& a : elements) { out_deg[a] = 0; in_deg[a] = 0; }
        for (const auto& [a, b] : res.pairs) {
            out_deg[a]++;
            in_deg[b]++;
        }

        bool all_exactly_one = true;
        bool all_at_most_one = true;
        for (const auto& a : elements) {
            if (out_deg[a] != 1) all_exactly_one = false;
            if (out_deg[a] > 1)  all_at_most_one  = false;
        }

        bool all_in_at_most_one = true;
        bool all_in_at_least_one = true;
        for (const auto& b : elements) {
            if (in_deg[b] > 1)  all_in_at_most_one  = false;
            if (in_deg[b] < 1)  all_in_at_least_one = false;
        }

        auto& m = res.mapping;
        m.is_function   = all_exactly_one && !res.pairs.empty();
        m.is_partial_fn = all_at_most_one;
        m.injective     = all_in_at_most_one && !res.pairs.empty();
        m.surjective    = all_in_at_least_one && !res.pairs.empty();
        m.bijective     = m.is_function && m.injective && m.surjective;

        if (m.bijective)
            m.summary = "Bijection on A: each element has exactly one image and exactly one preimage.";
        else if (m.is_function && m.injective)
            m.summary = "Injective function (one-to-one): distinct inputs map to distinct outputs.";
        else if (m.is_function && m.surjective)
            m.summary = "Surjective function (onto): every element of A appears as an image.";
        else if (m.is_function)
            m.summary = "Function f: A → A (each domain element has exactly one image).";
        else if (m.is_partial_fn && !res.pairs.empty())
            m.summary = "Partial function: no element is the first component of more than one pair.";
        else if (!res.pairs.empty())
            m.summary = "Not a function: some element is the first component of two or more pairs.";
        else
            m.summary = "Empty relation is vacuously a partial function.";
    }

    static std::string _format_pairs(const PairVec& pairs, size_t max_show = 24) {
        if (pairs.empty()) return "∅";
        std::string s;
        for (size_t i = 0; i < pairs.size() && i < max_show; ++i) {
            if (i) s += ", ";
            s += "(" + pairs[i].first + "," + pairs[i].second + ")";
        }
        if (pairs.size() > max_show)
            s += " … (+" + std::to_string(pairs.size() - max_show) + " more)";
        return s;
    }

    static void _compute_operations(
        AnalysisResult& res,
        const Set& elements,
        const PairVec& pairs,
        const std::set<Pair>& pair_set)
    {
        auto& op = res.operations;
        op.inverse             = compute_inverse(pairs);
        op.complement          = compute_complement(elements, pairs);
        op.compose_rr          = compute_compose(pairs, pairs);
        op.reflexive_closure   = compute_reflexive_closure(elements, pairs);
        op.symmetric_closure   = compute_symmetric_closure(pairs);
        op.transitive_closure  = compute_transitive_closure(elements, pairs);
        op.rr_subset_r         = is_subset(pair_set_from_vec(op.compose_rr), pair_set);
    }

    static void _compute_hasse(
        AnalysisResult& res,
        const Set& elements,
        std::function<bool(const Element&, const Element&)> has)
    {
        auto& h = res.hasse;
        auto& p = res.props;
        h.is_partial_order = p.reflexive && p.antisymmetric && p.transitive;

        if (!h.is_partial_order) {
            h.explanation = "A Hasse diagram is defined for a partial order, so R must be reflexive, antisymmetric, and transitive first.";
            return;
        }

        h.explanation = "Reflexive loops and transitive edges are omitted; each line is a cover relation.";

        for (const auto& a : elements) {
            for (const auto& b : elements) {
                if (a == b || !has(a, b)) continue;

                bool covered = true;
                for (const auto& c : elements) {
                    if (c == a || c == b) continue;
                    if (has(a, c) && has(c, b)) {
                        covered = false;
                        break;
                    }
                }
                if (covered) h.covers.push_back({a, b});
            }
        }

        std::map<Element, int> rank;
        for (const auto& e : elements) rank[e] = 0;
        for (size_t pass = 0; pass < elements.size(); ++pass) {
            bool changed = false;
            for (const auto& [a, b] : h.covers) {
                int next = rank[a] + 1;
                if (next > rank[b]) {
                    rank[b] = next;
                    changed = true;
                }
            }
            if (!changed) break;
        }

        int max_rank = 0;
        for (const auto& [_, level] : rank)
            max_rank = std::max(max_rank, level);
        h.levels.assign(max_rank + 1, Set{});
        for (const auto& e : elements)
            h.levels[rank[e]].push_back(e);

        for (const auto& a : elements) {
            bool has_lower = false;
            bool has_upper = false;
            for (const auto& b : elements) {
                if (a == b) continue;
                if (has(b, a)) has_lower = true;
                if (has(a, b)) has_upper = true;
            }
            if (!has_lower) h.minimal.push_back(a);
            if (!has_upper) h.maximal.push_back(a);
        }

        for (const auto& a : h.minimal) {
            if (std::all_of(elements.begin(), elements.end(),
                    [&](const Element& x){ return has(a, x); })) {
                h.least = a;
                break;
            }
        }
        for (const auto& a : h.maximal) {
            if (std::all_of(elements.begin(), elements.end(),
                    [&](const Element& x){ return has(x, a); })) {
                h.greatest = a;
                break;
            }
        }

        for (size_t i = 0; i < elements.size(); ++i) {
            for (size_t j = i + 1; j < elements.size(); ++j) {
                const auto& a = elements[i];
                const auto& b = elements[j];
                if (!has(a, b) && !has(b, a))
                    h.incomparable.push_back({a, b});
            }
        }
    }

    static void _build_inferences(
        AnalysisResult& res,
        const Set& elements,
        std::function<bool(const Element&, const Element&)> has)
    {
        auto& p  = res.props;
        auto& sp = res.special;

        // Equivalence classes
        if (p.reflexive && p.symmetric && p.transitive && !sp.empty) {
            std::set<Element> visited;
            std::vector<std::string> classes;
            for (const auto& a : elements) {
                if (visited.count(a)) continue;
                std::string cls = "{";
                bool first = true;
                for (const auto& x : elements) {
                    if (has(a, x) && has(x, a)) {
                        visited.insert(x);
                        if (!first) cls += ",";
                        cls += x;
                        first = false;
                    }
                }
                cls += "}";
                classes.push_back(cls);
            }
            std::string body = "R partitions A into " +
                std::to_string(classes.size()) + " class(es): ";
            for (size_t i = 0; i < classes.size(); ++i) {
                if (i) body += ", ";
                body += classes[i];
            }
            body += ". Quotient set A/R = { ";
            for (size_t i = 0; i < classes.size(); ++i) {
                if (i) body += ", ";
                body += classes[i];
            }
            body += " }.";
            res.inferences.push_back({"Equivalence classes", body});
        }

        // Hasse covering pairs
        if (p.reflexive && p.antisymmetric && p.transitive) {
            std::vector<std::string> covering;
            for (const auto& a : elements) {
                for (const auto& b : elements) {
                    if (a == b || !has(a, b)) continue;
                    bool is_covering = true;
                    for (const auto& c : elements) {
                        if (c == a || c == b) continue;
                        if (has(a, c) && has(c, b)) { is_covering = false; break; }
                    }
                    if (is_covering) covering.push_back(a + "≺" + b);
                }
            }
            std::string body = covering.empty()
                ? "No proper covering pairs (trivial / identity poset)."
                : "Covering pairs: ";
            for (size_t i = 0; i < covering.size(); ++i) {
                if (i) body += ", ";
                body += covering[i];
            }
            if (!covering.empty())
                body += ". Draw edges upward for each covering pair; omit (a,a) and implied transitive edges.";
            res.inferences.push_back({"Hasse diagram", body});
        }

        // Missing transitive pairs
        if (!p.transitive && !sp.empty) {
            std::string body = "R is not transitive. Missing at least: ";
            std::vector<std::string> missing;
            for (const auto& a : elements) {
                for (const auto& b : elements) {
                    for (const auto& c : elements) {
                        if (has(a,b) && has(b,c) && !has(a,c) && missing.size() < 3)
                            missing.push_back("(" + a + "," + c + ")");
                    }
                }
            }
            for (size_t i = 0; i < missing.size(); ++i) {
                if (i) body += ", ";
                body += missing[i];
            }
            body += ". Use Warshall's algorithm (O(n^3)) to compute transitive closure R+.";
            res.inferences.push_back({"Transitive closure needed", body});
        }

        // Reflexive closure hint
        if (!p.reflexive && !sp.empty && res.n > 0) {
            std::string body = "Add the diagonal pairs: ";
            for (size_t i = 0; i < std::min(p.refl_violations.size(), size_t(4)); ++i) {
                if (i) body += ", ";
                body += p.refl_violations[i];
            }
            if (p.refl_violations.size() > 4) body += "...";
            body += " to obtain the reflexive closure.";
            res.inferences.push_back({"Reflexive closure", body});
        }

        // Neither sym nor antisym
        if (!p.symmetric && !p.antisymmetric) {
            res.inferences.push_back({
                "Neither symmetric nor antisymmetric",
                "R has both symmetric and asymmetric off-diagonal pairs. "
                "It cannot be an equivalence relation or a partial order without modification."
            });
        }

        // Special cases
        if (sp.empty) {
            res.inferences.push_back({
                "Empty relation (R = emptyset)",
                "Vacuously symmetric, antisymmetric, and transitive. "
                "It is a strict partial order (irreflexive + transitive)."
            });
        }
        if (sp.universal) {
            res.inferences.push_back({
                "Universal relation (R = A x A)",
                "Every element relates to every other. "
                "It is an equivalence relation with a single equivalence class A. "
                "It is also total."
            });
        }
        if (sp.identity) {
            res.inferences.push_back({
                "Identity relation (Delta_A)",
                "R = {(a,a) | a in A}. Simultaneously an equivalence relation "
                "(each class is a singleton) and a partial order. "
                "It is the smallest reflexive relation on A."
            });
        }

        auto& mp = res.mapping;
        if (!mp.summary.empty() && !sp.empty) {
            res.inferences.push_back({ "Mapping classification", mp.summary });
        }

        if (!sp.empty && res.n > 0) {
            auto& op = res.operations;
            res.inferences.push_back({
                "Inverse R⁻¹",
                "R⁻¹ = " + _format_pairs(op.inverse) + ". Swaps each ordered pair (a,b) to (b,a)."
            });
            res.inferences.push_back({
                "Composition R ∘ R",
                "R² = R∘R = " + _format_pairs(op.compose_rr) + ". "
                + (op.rr_subset_r
                    ? "R² ⊆ R — consistent with transitivity."
                    : "R² ⊄ R — pairs in R² not in R indicate missing transitive links.")
            });
            if (!p.reflexive)
                res.inferences.push_back({
                    "Reflexive closure r(R)",
                    "r(R) = R ∪ Δ_A = " + _format_pairs(op.reflexive_closure)
                });
            if (!p.symmetric)
                res.inferences.push_back({
                    "Symmetric closure s(R)",
                    "s(R) = R ∪ R⁻¹ = " + _format_pairs(op.symmetric_closure)
                });
            if (!p.transitive)
                res.inferences.push_back({
                    "Transitive closure t(R)",
                    "t(R) = R⁺ (Warshall) = " + _format_pairs(op.transitive_closure)
                });
        }
    }
};

/** High-level API: parse strings and return JSON for the web bridge. */
inline std::string analyse_json(const std::string& set_raw, const std::string& pairs_raw) {
    Set elements = parse_set_validated(set_raw);
    PairVec pairs = parse_pairs(pairs_raw, elements);
    AnalysisResult r = RelationEngine::analyse(elements, pairs);
    return RelationEngine::to_json(r);
}

} // namespace math
