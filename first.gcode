; GRBL 1.1h
; Units: mm
; Absolute positioning

G21
G90

G0 X0 Y0

; Make sure Pen is up
G0 Z30

; Move to circle start point
G0 X50 Y30

; Drop the pen
G0 Z-5

; Set feed rate
F1000

; Draw full circle
G2 X50 Y30 I-20 J0

; Lift the pen
G0 Z30

; Return home
G0 X0 Y0