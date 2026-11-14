# id: neg
# kind: cart
# window: 5000-5FFF
; the displacement 03FF is the largest (the negative case asm-branch-range is one more)
        ORG   $5000
        B     far
        ORG   $5401
far:    HLT
