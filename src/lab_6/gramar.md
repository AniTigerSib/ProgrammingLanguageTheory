I.

    S -> I(E)

II.

    E -> -E|+(T)|*(T)|S|M
    T -> E,T|E
    M -> I|C

III.

    I -> AK|A
    K -> DK|D

IV.

    C -> #R
    R -> DR|D

V.

    A -> а|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|_

VI.

    D -> 0|1

Grammar:

    S -> I(E)
    E -> -E|+(T)|*(T)|#R|I(E)|I
    T -> E,T|E
    // M -> I|C
    I -> AK|A
    K -> DK|D
    // C -> #R
    R -> DR|D
    A -> а|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z|_
    D -> 0|1

сначала разобрать выражение в скобках, потом присваивать

## Изменения

    S -> I(E)
    S -> I[E]

    E -> -E|+(T)|*(T)|S|M
    E -> -(T)|+(T)|*(T)|S|M

    T -> E,T|E
    T -> T,E|E