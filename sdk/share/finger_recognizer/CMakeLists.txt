include_directories(
    .
    )
    
add_library(finger_recognizer STATIC
    finger_extract.c
    finger_recognizer.c
    fr_minutiae_fix.c
    fr_minutiaeMatch.c
    circle.c
    gaussian.c
    oneCountTable.c
    sinTable.c
    sqrtTable.c
    )
