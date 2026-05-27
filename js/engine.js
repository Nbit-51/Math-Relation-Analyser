/**
 * engine.js — Loads the C++ / WebAssembly analysis engine (sole compute backend)
 */

(function (global) {
  'use strict';

  let wasmModule = null;
  let wasmAnalyse = null;
  let ready = false;
  let initError = null;
  let initPromise = null;

  function normaliseResult(json) {
    if (json.error) throw new Error(json.error);
    const result = {
      elements: json.elements,
      pairs: json.pairs || [],
      n: json.elements.length,
      matrix: json.matrix,
      props: {
        refl: json.props.refl,
        irrefl: json.props.irrefl,
        sym: json.props.sym,
        antisym: json.props.antisym,
        asym: json.props.asym,
        trans: json.props.trans,
        total: json.props.total,
        reflViol: json.props.reflViol || [],
        irreflViol: json.props.irreflViol || [],
        symViol: json.props.symViol || [],
        antisymViol: json.props.antisymViol || [],
        transViol: json.props.transViol || [],
      },
      special: json.special,
      mapping: {
        isFunction: json.mapping?.isFunction ?? false,
        isPartialFn: json.mapping?.isPartialFn ?? false,
        injective: json.mapping?.injective ?? false,
        surjective: json.mapping?.surjective ?? false,
        bijective: json.mapping?.bijective ?? false,
        summary: json.mapping?.summary ?? '',
      },
      classifications: (json.classifications || []).map(c => ({
        label: c.label,
        cls: c.cls,
      })),
      inferences: json.inferences || [],
      vacuous: json.special?.empty ?? false,
      operations: {
        inverse: json.operations?.inverse || [],
        complement: json.operations?.complement || [],
        composeRR: json.operations?.composeRR || [],
        reflexiveClosure: json.operations?.reflexiveClosure || [],
        symmetricClosure: json.operations?.symmetricClosure || [],
        transitiveClosure: json.operations?.transitiveClosure || [],
        rrSubsetR: json.operations?.rrSubsetR ?? false,
      },
      hasse: {
        isPartialOrder: json.hasse?.isPartialOrder ?? false,
        covers: json.hasse?.covers || [],
        levels: json.hasse?.levels || [],
        minimal: json.hasse?.minimal || [],
        maximal: json.hasse?.maximal || [],
        least: json.hasse?.least || '',
        greatest: json.hasse?.greatest || '',
        incomparable: json.hasse?.incomparable || [],
        explanation: json.hasse?.explanation || '',
      },
    };
    return result;
  }

  function loadScript(src) {
    return new Promise((resolve, reject) => {
      const script = document.createElement('script');
      script.src = src;
      script.async = true;
      script.onload = resolve;
      script.onerror = () => reject(new Error(`Failed to load ${src}`));
      document.head.appendChild(script);
    });
  }

  async function init() {
    if (initPromise) return initPromise;

    initPromise = (async () => {
      try {
        if (typeof global.createRelationEngine !== 'function') {
          await loadScript('wasm/relation_engine.js');
        }
        if (typeof global.createRelationEngine !== 'function') {
          throw new Error('WASM loader not found. Run build-wasm.bat first.');
        }

        wasmModule = await global.createRelationEngine({
          locateFile: (path) => `wasm/${path}`,
        });
        wasmAnalyse = wasmModule.cwrap('analyse', 'string', ['string', 'string']);
        ready = true;
        initError = null;
        return true;
      } catch (err) {
        ready = false;
        initError = err;
        throw err;
      }
    })();

    return initPromise;
  }

  function isReady() {
    return ready;
  }

  function getInitError() {
    return initError;
  }

  async function analyse(setRaw, pairsRaw) {
    if (!ready) await init();
    const jsonStr = wasmAnalyse(setRaw ?? '', pairsRaw ?? '');
    return normaliseResult(JSON.parse(jsonStr));
  }

  global.RelationEngine = { init, analyse, isReady, getInitError };
})(window);
