import json
import random
import csv

def decrypt_apriltag(tag_value):
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
        return None # Return None instead of erroring since we know the tags are mostly valid
        
    return {"order": order, "x": x, "y": y, "key": k}


def generate_set_from_json(json_filename="tags_grouped_by_order.json"):
    print(f"1. Loading pre-sorted tags from {json_filename}...")
    
    # Load the JSON data
    try:
        with open(json_filename, "r") as file:
            raw_tags_by_order = json.load(file)
    except FileNotFoundError:
        print(f"Error: Could not find '{json_filename}'. Make sure it's in the same folder.")
        return

    # Prepare our usable dictionary by quickly decoding the loaded tags to get X, Y, and Key ID
    tags_by_order = {i: [] for i in range(1, 15)}
    
    for order_str, tag_list in raw_tags_by_order.items():
        order_int = int(order_str)
        for tag_value in tag_list:
            result = decrypt_apriltag(tag_value)
            if result:
                result["tag"] = tag_value 
                tags_by_order[order_int].append(result)

    # Shuffle the tags to ensure a random output every time
    for order in tags_by_order:
        random.shuffle(tags_by_order[order])

    print("2. Searching for a set with unique X/Y coordinates and balanced Key IDs...")
    
    def solve(current_order, used_x, used_y, key_counts, current_selection):
        if current_order > 14:
            return current_selection

        for item in tags_by_order[current_order]:
            x, y, k = item["x"], item["y"], item["key"]

            # Constraints: Unique X, Unique Y, Max 3 of the same Key ID
            if x not in used_x and y not in used_y and key_counts[k] < 3:
                
                used_x.add(x)
                used_y.add(y)
                key_counts[k] += 1
                current_selection.append(item)

                result = solve(current_order + 1, used_x, used_y, key_counts, current_selection)
                if result is not None:
                    return result 

                used_x.remove(x)
                used_y.remove(y)
                key_counts[k] -= 1
                current_selection.pop()

        return None 

    key_counts = {0: 0, 1: 0, 2: 0, 3: 0, 4: 0}
    final_set = solve(1, set(), set(), key_counts, [])

    if final_set:
        print("\n--- Success! Found Valid Tag Set ---")
        print(f"{'Order':<6} | {'Tag ID':<8} | {'Key':<4} | {'X':<4} | {'Y':<4}")
        print("-" * 40)
        for item in final_set:
            print(f"{item['order']:<6} | {item['tag']:<8} | {item['key']:<4} | {item['x']:<4} | {item['y']:<4}")
        
        # Export to CSV
        csv_filename = "final_apriltag_set.csv"
        with open(csv_filename, mode='w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["Order", "Tag ID", "Key ID", "X", "Y"])
            for item in final_set:
                writer.writerow([item['order'], item['tag'], item['key'], item['x'], item['y']])
        
        print(f"\nSaved successfully to: {csv_filename}")
    else:
        print("Failed to find a combination that satisfies all rules.")

if __name__ == "__main__":
    # Make sure the filename matches what you generated previously!
    generate_set_from_json("tags_grouped_by_order.json")