import cv2
import time
import numpy as np
import global_states
import ares_utils as ares


def detect_line_y(img):
    """
    Processes the image, finds the line, draws overlays, 
    and returns the average Y coordinate of the detected line.
    """
    height, width = img.shape[:2]
    
    # ROI
    roi_start_y = int(height * global_states.TOP_CROP_RATIO)
    roi_end_y = height - roi_start_y
    roi_start_x = int(width * global_states.SIDE_CROP_RATIO)
    roi_end_x = width - roi_start_x
        
    roi_img = img[roi_start_y:roi_end_y, roi_start_x:roi_end_x]
    roi_height, roi_width = roi_img.shape[:2]
        
    # Mask
    # Convert ROI to HSV
    hsv_roi = cv2.cvtColor(roi_img, cv2.COLOR_BGR2HSV)
    
    # Define the HSV range for white
    lower_white = np.array([0, 0, 200])
    upper_white = np.array([180, 30, 255])
    
    # Create the mask
    white_mask = cv2.inRange(hsv_roi, lower_white, upper_white)
    
    # Horizontal segmentation
    segment_width = roi_width // global_states.NUM_SEGMENTS
    line_points = [] 
    
    for i in range(global_states.NUM_SEGMENTS):
        local_x_start = i * segment_width
        local_x_end = (i + 1) * segment_width if i < global_states.NUM_SEGMENTS - 1 else roi_width
        segment_mask = white_mask[:, local_x_start:local_x_end]
        
        M = cv2.moments(segment_mask)
        area = M["m00"]
        
        if area > global_states.MIN_AREA_THRESHOLD:
            local_cX = int(M["m10"] / area)
            local_cY = int(M["m01"] / area)
            
            global_cX = local_cX + local_x_start + roi_start_x
            global_cY = local_cY + roi_start_y
            
            line_points.append((global_cX, global_cY))
            cv2.circle(img, (global_cX, global_cY), 5, (0, 255, 0), -1)
    
    # Visualization
    for i in range(len(line_points) - 1):
        cv2.line(img, line_points[i], line_points[i+1], (255, 0, 0), 3)
    cv2.rectangle(img, (roi_start_x, roi_start_y), (roi_end_x, roi_end_y), (255, 255, 0), 2)
    
    # Calculating avg y
    avg_y = None
    if len(line_points) >= 2:
        avg_y = sum([pt[1] for pt in line_points]) / len(line_points)
        
    return avg_y, img, white_mask
    
    

def calibrate_reference_line():
    """
    Captures the camera frame at the cell center, detects the line's Y-coordinate,
    and saves it to a global variable
    """
    RED_NEGATIVE_OFFSET = 30
    
    print("Capturing reference line at center position...")
    

    while global_states.GRID_CENTER_REFERENCE_Y is None:
        frame_bytes_floor = ares.get_camera_frame("floor")
      
        if frame_bytes_floor:
            arr = np.frombuffer(frame_bytes_floor, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
            
            if img is not None:
                avg_y, processed_img, mask = detect_line_y(img)
                
                if avg_y is not None:
                    
                    global_states.GRID_CENTER_REFERENCE_Y = avg_y
                    print(f"Grid Center Global Reference Y : {global_states.GRID_CENTER_REFERENCE_Y:.2f} pixels.")
                    
                    # Detecting min red value
                    height, width = img.shape[:2]
                    
                    # ROI
                    roi_start_y = int(height * global_states.TOP_CROP_RATIO)
                    roi_end_y = height - roi_start_y
                    roi_start_x = int(width * global_states.SIDE_CROP_RATIO)
                    roi_end_x = width - roi_start_x
                   
                    roi_img = img[roi_start_y:roi_end_y, roi_start_x:roi_end_x]
                    
                    hsv_roi = cv2.cvtColor(roi_img, cv2.COLOR_BGR2HSV)
                    
                    lower_red_1 = np.array([0, 100, 100])
                    upper_red_1 = np.array([10, 255, 255])
                    lower_red_2 = np.array([160, 100, 100])
                    upper_red_2 = np.array([180, 255, 255])
                    
                    mask1 = cv2.inRange(hsv_roi, lower_red_1, upper_red_1)
                    mask2 = cv2.inRange(hsv_roi, lower_red_2, upper_red_2)
                    red_mask = cv2.bitwise_or(mask1, mask2)
                    
                    # Find all Y, X coordinates where the red mask is active 
                    y_coords, x_coords = np.where(red_mask > 0)
                    
                    if len(y_coords) > 0:
                        min_y_local = np.min(y_coords)
                        
                        min_y_global = min_y_local + roi_start_y
                        
                        
                        global_states.MOVE_RELATIVE_REFERENCE_Y = min_y_global - RED_NEGATIVE_OFFSET
                    else:
                        print("Warning: No red detected! Using fallback offset.")
                        global_states.MOVE_RELATIVE_REFERENCE_Y = global_states.GRID_CENTER_REFERENCE_Y + 80

                    print(f"Move relative Global Reference Y : {global_states.MOVE_RELATIVE_REFERENCE_Y:.2f} pixels.")
                    
                    # Draw the GRID_CENTER_REFERENCE_Y line
                    cv2.line(processed_img, (0, int(global_states.GRID_CENTER_REFERENCE_Y)), 
                             (width, int(global_states.GRID_CENTER_REFERENCE_Y)), (255, 0, 255), 2)
                    
                    # Draw the new MOVE_RELATIVE_REFERENCE_Y line
                    cv2.line(processed_img, (0, int(global_states.MOVE_RELATIVE_REFERENCE_Y)), 
                             (width, int(global_states.MOVE_RELATIVE_REFERENCE_Y)), (0, 255, 255), 2)
                             
                    cv2.imshow("Reference Capture", processed_img)
                    
                    cv2.waitKey(500) 
                    cv2.destroyWindow("Reference Capture")
                    
                    return global_states.GRID_CENTER_REFERENCE_Y 
                    
        time.sleep(0.1)

def is_guard_bot_present():
    """
    Takes a single snapshot from the front cameras to check if the 
    hostile robot is within the danger zone.
    Returns True if hostile is too close, False otherwise.
    """
    
    WARNING_LINE_Y = 300
    
    frame_bytes_left = ares.get_camera_frame("front_left")
    frame_bytes_right = ares.get_camera_frame("front_right")
    
    if frame_bytes_left and frame_bytes_right:
        
        arr_left = np.frombuffer(frame_bytes_left, dtype=np.uint8)
        img_left = cv2.imdecode(arr_left, cv2.IMREAD_COLOR) 
        
        arr_right = np.frombuffer(frame_bytes_right, dtype=np.uint8)
        img_right = cv2.imdecode(arr_right, cv2.IMREAD_COLOR)
        
        if img_left is not None and img_right is not None:
            
            # Concatenate
            combined_img = cv2.hconcat([img_left, img_right])
            
            
            height, width = combined_img.shape[:2]
            frame_center_x = width // 2  

            
            cv2.line(combined_img, (0, WARNING_LINE_Y), (width, WARNING_LINE_Y), (255, 0, 255), 2)

            # Convert to HSV and mask
            hsv = cv2.cvtColor(combined_img, cv2.COLOR_BGR2HSV)
            lower_green = np.array([40, 50, 50])
            upper_green = np.array([80, 255, 255])
            green_mask = cv2.inRange(hsv, lower_green, upper_green)
            
        
            M = cv2.moments(green_mask)
            area = M["m00"]
            
            hostile_too_close = False 
            
            if area > 1000: # Noise filter
                
                cX = int(M["m10"] / area)
                cY = int(M["m01"] / area)
                offset_x = cX - frame_center_x
                
                
                cv2.circle(combined_img, (cX, cY), 5, (0, 0, 255), -1)
                cv2.line(combined_img, (frame_center_x, cY), (cX, cY), (255, 0, 0), 2)
                
            
                if cY > WARNING_LINE_Y:
                    print(f"WARNING: Hostile robot crossed proximity line! Depth: {cY}")
                    hostile_too_close = True
                else:
                    print(f"Hostile visible but at a safe distance. Depth: {cY}")
            
            
            cv2.imshow("Ares Panoramic Vision", combined_img)
            cv2.waitKey(1) 
            
            return hostile_too_close

        else:
            print("Error: Could not decode one or both camera frames.")
            return False
            
    else:
        print("Failed to get frames. API might be down.")
        return False


