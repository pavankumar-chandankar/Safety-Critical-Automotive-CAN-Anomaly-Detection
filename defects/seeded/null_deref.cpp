// TRAINING MUTANT ONLY. Intentionally wrong: never link into the production target.
int deliberately_bad_null_deref(int* pointer) {
  if(pointer == nullptr) {
    return *pointer; // Expected Clang Static Analyzer core.NullDereference.
  }
  return *pointer;
}
