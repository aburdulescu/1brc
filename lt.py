for c in range(256):
    if c >= ord('0') and c <= ord('9'):
        print("{10, %d}," % (c - ord('0')))
    else:
        print("{1, 0},")
