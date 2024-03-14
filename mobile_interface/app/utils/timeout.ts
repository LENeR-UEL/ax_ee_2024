export function timeout(delay: number): Promise<void> {
  return new Promise(resolve => {
    setTimeout(resolve, delay);
  });
} 

export function timeoutSync(delay: number) {
  const start = Date.now();

  while (Date.now() - start < delay) {}
}