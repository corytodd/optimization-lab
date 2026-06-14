# Reference

## Quality

```bash
# check formatting
cmake --build build --target format

# apply fixes in place
cmake --build build --target format-fix

# run clang-tidy
cmake --build build --target tidy

# Testing
cmake --build build && ctest --test-dir build --output-on-failure
```
