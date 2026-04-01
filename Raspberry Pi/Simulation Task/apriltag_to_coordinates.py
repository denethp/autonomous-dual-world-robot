import ares_utils as ares

def decrypt_apriltag(tag_value):
    # Convert input to a string and pad with zeros in front to get 5 digit value
    tag_str = str(tag_value).zfill(5)
        
    # Extract Key ID
    k = int(tag_str[0])

    # Determining the fuction based on Key ID
    if k == 0:
        K = 6180

        # Extract payload 
        payload = tag_str[1:] 
        p_rev = int(payload[::-1]) 
        
        # Calculate A 
        A = ((p_rev * 7) + K) % 10000

    elif k == 1:
        K = 3141

        # Extract payload 
        payload = tag_str[1:]      
        p_swap = int(payload[2:] + payload[:2])
        
        # Calculate A 
        A = ((p_swap * 3) + K) % 8750
        
    elif k == 2:
        K = 2718

        # Extract payload 
        payload = tag_str[1:]   
        p_comp = 9999 - int(payload) 

        # Calculate A 
        A = ((p_comp * 9) + K) % 8750
        
    elif k == 3:
        K = 8080

        # Extract payload 
        payload = tag_str[1:]   
        p_int = int(payload[-1] + payload[1:3] + payload[0]) 


        # Calculate A 
        A = ((p_int * 11) + K) % 8750
        
    elif k == 4:
        K = 4040

        # Extract payload 
        payload = tag_str[1:]   
        graycode = int(payload) ^ (int(payload) // 2)

        # Calculate A 
        A = graycode ^ K
        
    else:
        print("Invalid tag_value: Key ID must be between 0 and 4.")
        return None
    

    # Convert A into coordinates 
    order = (A // 625) + 1
    remainder = A % 625
    x = remainder // 25
    y = remainder % 25
            

    # Validate coordinate output ranges
    if not (1 <= order <= 14):
        raise ValueError(f"Decryption failed: 'order' ({order}) is out of valid range (1-14).")
    if not (0 <= x <= 24):
        raise ValueError(f"Decryption failed: 'x' ({x}) is out of valid range (0-24).")
    if not (0 <= y <= 24):
        raise ValueError(f"Decryption failed: 'y' ({y}) is out of valid range (0-24).")
        
    ares.post_apriltag(tag_str, order, x, y)
        
    return {
        "order": order,
        "x": x,
        "y": y,
    }
