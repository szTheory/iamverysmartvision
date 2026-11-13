# id: neg
# kind: cart
# window: 5000-5FFF
; the displacement 0400 is one above the largest (03FF): target = pc + 2 + 0400
        ORG   $5000
        B     far
        ORG   $5402
far:    HLT
