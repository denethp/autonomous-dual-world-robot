def coordinates_to_a(code: str) -> int:
    # Extracting Order, x, and y
    order = int(code[:2])
    x = int(code[2:4])
    y = int(code[4:])

    # Calculating A
    A = ((order - 1) * 625) + (x * 25) + y
    
    return A

def find_clue(A : int, tag_value : int) -> int:
    # Convert input to a string and pad with zeros in front to get 5 digit value
    tag_str = str(tag_value).zfill(5)
            
    # Extract payload 
    payload = tag_str[1:]   
    # Reverse payload and convert to integer   
    p_rev = int(payload[::-1])

    # Calculate K 
    K = (A - (p_rev * 7)) % 10000

    return K

print(find_clue(coordinates_to_a("012310"), 5194))
print(find_clue(coordinates_to_a("071204"), 2893))