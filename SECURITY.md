# Security Policy

## Supported Versions

| Version         | Supported          |
| --------------- | ------------------ |
| 0.5.x           | :white_check_mark: |
| < 0.5.0         | :x:                |

## Reporting a Vulnerability

If you discover a security vulnerability or critical numerical flaw that compromises safety guarantees:

1. Please do not open a public issue.
2. Use GitHub's private vulnerability reporting on this repository
   (Security -> Report a vulnerability), which reaches the maintainers without
   disclosing anything publicly.
3. Include a minimal reproducing snippet, the compiler and standard library, and
   which backend was in use -- native or `FIXEDWIDE_FORCE_PORTABLE`.

A "critical numerical flaw" here means a wrong answer returned as success: an
overflow that is not reported, a rounding that does not match the mode asked
for, or a parse that accepts input it cannot represent exactly. Those are
treated as security issues because callers rely on the return type to be
truthful.
