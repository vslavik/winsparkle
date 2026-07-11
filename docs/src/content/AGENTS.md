# Documentation Editing Instructions

- For C API reference pages, use the public header comments in `include/winsparkle.h` as the source text.
- Do not remove information from the header comments when converting them to docs.
- You may fix typos, bad English, or awkward phrasing, but preserve the original meaning and all documented details.
- Preserve semantic markup from the header comments. For example, convert `@note` blocks to Starlight note markup instead of flattening them into ordinary paragraphs.
- Preserve important `@see` references as explicit links in the docs.
- Do not use backticks in section headings.
- Function link text must include trailing parentheses, for example `[win_sparkle_cleanup()](#win_sparkle_cleanup)`.
- Use the <Since> component to indicate the version in which a function was added, for example `<Since version="0.9" />`. `<Since>` must be the last thing in a function's docs text.
- Use <ApiFunction /> for function names in headings, for example `### <ApiFunction /> win_sparkle_cleanup()`.
