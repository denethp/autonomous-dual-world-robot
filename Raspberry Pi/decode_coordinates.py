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
        # Reverse payload and convert to integer   
        p_rev = int(payload[::-1]) 
        
        # Calculate A 
        A = ((p_rev * 7) + K) % 10000

    elif k == 1:
        K = 3141

        # Extract payload 
        payload = tag_str[1:]   
        # Reverse payload and convert to integer   
        p_swap = int(payload[2:] + payload[:2])
        
        # Calculate A 
        A = ((p_swap * 3) + K) % 8750
        
    elif k == 2:
        pass
    elif k == 3:
        pass
    elif k == 4:
        pass
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
        
    return {
        "order": order,
        "x": x,
        "y": y,
    }

scanned_tag = 5194 
result = decrypt_apriltag(scanned_tag)
print(result)

scanned_tag = 2893 
result = decrypt_apriltag(scanned_tag)
print(result)

scanned_tag = 18862
result = decrypt_apriltag(scanned_tag)
print(result)

scanned_tag = 16722
result = decrypt_apriltag(scanned_tag)
print(result)