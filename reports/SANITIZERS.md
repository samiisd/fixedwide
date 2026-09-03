# Sanitizer Verification Report: fixedwide 0.5.0-alpha.3

## 1. Setup & Environment
- **Compiler**: Clang 22.1.8
- **Build Type**: Debug with `-DFIXEDWIDE_SANITIZE=ON`
- **Compiler Flags**: `-fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all`
- **Linker Flags**: `-fsanitize=address,undefined`

## 2. Test Execution
Executed command:
```bash
ctest --test-dir build-asan --output-on-failure
```

## 3. Results Summary

| Test Index | Target | Sanitizer Diagnostics | Exit Status |
| :---: | :--- | :---: | :---: |
| 1 | `fixedwide.storage` | 0 | PASSED |
| 2 | `fixedwide.wide` | 0 | PASSED |
| 3 | `fixedwide.rounding` | 0 | PASSED |
| 4 | `fixedwide.core` | 0 | PASSED |
| 5 | `fixedwide.mixed` | 0 | PASSED |
| 6 | `fixedwide.chars` | 0 | PASSED |
| 7 | `fixedwide.binary` | 0 | PASSED |
| 8 | `fixedwide.floating` | 0 | PASSED |
| 9 | `fixedwide.no_alloc` | 0 | PASSED |
| 10 | `fixedwide.signed_min` | 0 | PASSED |
| 11 | `fixedwide.audit.audit_same_domain` | 0 | PASSED (12.5s) |
| 12 | `fixedwide.audit.audit_mixed` | 0 | PASSED (16.0s) |
| 13 | `fixedwide.audit.audit_known` | 0 | PASSED |
| 14 | `fixedwide.audit.audit_floating` | 0 | PASSED |
| 15 | `fixedwide.audit.audit_io` | 0 | PASSED |
| 16 | `fixedwide.audit.audit_text_targeted` | 0 | PASSED |
| 17 | `fixedwide.audit.audit_portable_edges` | 0 | PASSED |
| 18 | `fixedwide.audit.audit_portable_div128` | 0 | PASSED |
| 19-22 | `fixedwide.negative.*` (compile-fail) | N/A | PASSED |
| 23 | `fixedwide.oracle` (Boost.Multiprecision) | 0 | PASSED (0.36s) |

**Total Sanitizer Violations Detected**: **0**
