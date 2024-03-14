/**
 * run has two primary use cases:
 * IIFE (Immediately Invoked Function Expression)
 * do expression (currently Stage 1 Proposal)
 *
 * When acting like an IIFE, run takes a sync or async function and runs it.
 *
 * @link https://maxgreenwald.me/blog/do-more-with-run
 */
export function run<T>(fn: () => T): T {
  return fn();
}
