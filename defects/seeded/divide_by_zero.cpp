// TRAINING MUTANT ONLY. Deliberately wrong: never link into production target.
int deliberately_bad_division(int number, int divisor) {
  if(divisor == 0) {
    return number / divisor; // Expected Clang Static Analyzer core.DivideZero.
  }
  return number / divisor;
}
