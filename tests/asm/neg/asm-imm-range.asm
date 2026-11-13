# id: neg
# kind: cart
# window: 5000-5FFF
; even with SDBD an immediate is at most FFFF
        ORG   $5000
        SDBD
        MVII  #$FFFF+1, R0
