// TRAINING MUTANT ONLY. Deliberately wrong: never link into production target.
int deliberately_bad_array_lookup(int input) {
  int array[3] = {1,2,3};
  if(input == 3) {
    return array[input]; // Expected array bounds diagnostic.
  }
  return array[0];
}
