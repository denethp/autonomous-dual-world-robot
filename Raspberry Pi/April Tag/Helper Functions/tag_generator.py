import random

def decrypt_apriltag(tag_value):
    # Convert input to a string and pad with zeros in front to get 5 digit value
    tag_str = str(tag_value).zfill(5)
    k = int(tag_str[0])

    if k == 0:
        K = 6180
        payload = tag_str[1:]   
        p_rev = int(payload[::-1]) 
        A = ((p_rev * 7) + K) % 10000
    elif k == 1:
        K = 3141
        payload = tag_str[1:]   
        p_swap = int(payload[2:] + payload[:2])
        A = ((p_swap * 3) + K) % 8750
    elif k == 2:
        K = 2718
        payload = tag_str[1:]   
        p_comp = 9999 - int(payload) 
        A = ((p_comp * 9) + K) % 8750
    elif k == 3:
        K = 8080
        payload = tag_str[1:]   
        p_int = int(payload[-1] + payload[1:3] + payload[0]) 
        A = ((p_int * 11) + K) % 8750
    elif k == 4:
        K = 4040
        payload = tag_str[1:]   
        graycode = int(payload) ^ (int(payload) // 2)
        A = graycode ^ K
    else:
        return None
    
    order = (A // 625) + 1
    remainder = A % 625
    x = remainder // 25
    y = remainder % 25
            
    if not (1 <= order <= 14) or not (0 <= x <= 24) or not (0 <= y <= 24):
        raise ValueError()
        
    return {"order": order, "x": x, "y": y, "key": k}


def generate_unique_tag_set():
    print("1. Scanning and grouping all valid tags (0 - 48713)...")
    tags_by_order = {i: [] for i in range(1, 15)}
    
    # Pre-calculate and group all valid tags up to 48713
    for tag_value in range(48714):
        try:
            result = decrypt_apriltag(tag_value)
            if result and 1 <= result["order"] <= 14:
                # Store the tag along with its decoded data
                result["tag"] = tag_value 
                tags_by_order[result["order"]].append(result)
        except ValueError:
            pass

    # Shuffle the tags within each order to ensure a random output every time
    for order in tags_by_order:
        random.shuffle(tags_by_order[order])

    print("2. Searching for a set with unique X/Y coordinates and balanced Key IDs...")
    
    # Backtracking function to find a valid combination
    def solve(current_order, used_x, used_y, key_counts, current_selection):
        # Base case: We successfully found 14 tags!
        if current_order > 14:
            return current_selection

        # Try every tag in the current order group
        for item in tags_by_order[current_order]:
            x, y, k = item["x"], item["y"], item["key"]

            # Check our constraints: Unique X, Unique Y, and Max 3 of the same Key ID
            if x not in used_x and y not in used_y and key_counts[k] < 3:
                
                # Make a choice
                used_x.add(x)
                used_y.add(y)
                key_counts[k] += 1
                current_selection.append(item)

                # Move to the next order
                result = solve(current_order + 1, used_x, used_y, key_counts, current_selection)
                if result is not None:
                    return result # Success! Bubble up the answer.

                # Undo the choice (Backtrack) if it led to a dead end
                used_x.remove(x)
                used_y.remove(y)
                key_counts[k] -= 1
                current_selection.pop()

        return None # No valid combination found on this path

    # Initialize tracking variables and start the solver at Order 1
    key_counts = {0: 0, 1: 0, 2: 0, 3: 0, 4: 0}
    final_set = solve(1, set(), set(), key_counts, [])

    if final_set:
        print("\n--- Success! Found Valid Tag Set ---")
        print(f"{'Order':<6} | {'Tag ID':<8} | {'Key':<4} | {'X':<4} | {'Y':<4}")
        print("-" * 40)
        for item in final_set:
            print(f"{item['order']:<6} | {item['tag']:<8} | {item['key']:<4} | {item['x']:<4} | {item['y']:<4}")
        
        print("\nKey ID Distribution:")
        for k, count in key_counts.items():
            print(f"Key {k}: {count} tags")
            
        # Return the data so visualize_tags.py can use it
        return final_set 
    else:
        print("Failed to find a combination that satisfies all rules.")
        return None


if __name__ == "__main__":
    # If you run this file directly, it will just print the results to the terminal
    generate_unique_tag_set()