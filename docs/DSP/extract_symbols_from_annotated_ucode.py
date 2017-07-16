# codec: utf8

import glob
import re

# git revert 5f0c892ed0c280eb7fd56e9e2789b41b4ae8ee5a to get annotated ucode files

# Extracts symbol names and locations from:
# - Normal assembly-style labels
# - Weird C-style void-returning function declarations
# Note that DSP_UC_SuperMarioGalaxy.txt has an entirely different style of annotation that uses comments :/
# These files also have a lot of other juicy information aside from the symbols.

for filename in glob.glob("DSP_*.txt"):
    print(filename)
    # open file
    with open(filename) as f:
        text = f.read()

    # turn multi-line comments into single-line comments
    text = re.sub(r"(?ms)/\*(.*?)\*/", lambda m: re.sub(r"(?m)^", "// ", m.group(1)) + "\n", text)

    # uncomment original assembly
    text = re.sub(r"(?m)(//|#)\s*([a-f0-9]{4}\s.*$)", r"\2", text)

    # remove remaining comments
    text = re.sub(r"(?m)(//|#).*$", "", text)

    # remove 'do {' lines
    text = re.sub(r"(?m)[^\w]do\s+\{", "", text)

    # iterate through matches
    position_expr = re.compile(r"(?m)\s*(?P<position>[a-f0-9]{4})\s+")
    symbol_expr = re.compile(r"(?ims)(^void\s+(?P<symbol>\w+).*?\{|^\s*(?P<label>\w+):\s*$)")
    start = 0
    while True:
        # find next symbol
        symbol_match = re.search(symbol_expr, text[start:])
        if not symbol_match:
            break
        symbol_name = symbol_match.group('symbol') or symbol_match.group('label')
        start += symbol_match.end()

        # find next position
        position_match = re.match(position_expr, text[start:])
        try:
            position = position_match.group('position')
        except:
            print((symbol_name, text[start:start+10]))
            raise
        print((position, symbol_name))
