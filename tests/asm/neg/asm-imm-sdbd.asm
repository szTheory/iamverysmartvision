# id: neg
# kind: cart
# window: 5000-5FFF
; a forward label above 03FF as an immediate needs SDBD in front (resolved in pass 2)
        ORG   $5000
        MVII  #far, R0
        ORG   $5400
far:    HLT
