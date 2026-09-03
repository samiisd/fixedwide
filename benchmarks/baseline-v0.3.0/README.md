# Unchanged fixedwide 0.3.0 runtime baseline

This is the unchanged runtime source, public headers, configuration and license
from release 0.3.0. It exists only to reproduce the before/after benchmarks, not
as another dependency or supported library target. Non-runtime tests, examples
and historical reports are omitted. `tools/rounding_compare.py` disables those
optional targets and builds this snapshot separately from the current library.

The same **current** rounding benchmark source is compiled once against each
library, with both rounding modes explicitly requested. The current library's
changed default arguments therefore cannot change this comparison's semantics.
