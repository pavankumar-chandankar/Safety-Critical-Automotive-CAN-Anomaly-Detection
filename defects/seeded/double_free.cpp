// TRAINING MUTANT ONLY. Deliberately wrong: never link into production target.
void deliberately_bad_double_delete() {
  int* ptr = new int(8);
  delete ptr;
  delete ptr; // Expected unix.Malloc double-delete/double-free diagnostic.
}
