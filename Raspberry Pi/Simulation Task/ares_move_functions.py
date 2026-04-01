import cv2
import math
import time
import numpy as np
import global_states
import ares_utils as ares
import ares_cv_functions as cv

def update_grid_position(steps, is_moving_forward=True):
    """
    Updates the global X, Y coordinates based on the current heading
    and the number of grids crossed.
    """
    modifier = steps if is_moving_forward else -steps
    
    # Heading 0: North (-Y), Heading 1: East (+X), Heading 2: South (+Y), Heading 3: West (-X)
    if global_states.CURRENT_HEADING == 0:
        global_states.CURRENT_Y -= modifier
    elif global_states.CURRENT_HEADING == 1:
        global_states.CURRENT_X += modifier
    elif global_states.CURRENT_HEADING == 2:
        global_states.CURRENT_Y += modifier
    elif global_states.CURRENT_HEADING == 3:
        global_states.CURRENT_X -= modifier

def go_to_grid_point_chunked(target_x, target_y):
    
    # No. of cells move in one leap
    MAX_STRIDE = 4
    
    print(f"--- Navigating to ({target_x}, {target_y}) in chunks of {MAX_STRIDE} ---")
    
    while global_states.CURRENT_X != target_x or global_states.CURRENT_Y != target_y:
        
        while cv.is_guard_bot_present():
            print("GUARD BOT DETECTED IN PATH! Evading backward 1 grid...")
            backward(4) 
            update_grid_position(steps=4, is_moving_forward=False)
            print(f"Fell back to: ({global_states.CURRENT_X}, {global_states.CURRENT_Y})")
            
            time.sleep(1.0) 
            
        delta_x = target_x - global_states.CURRENT_X
        delta_y = target_y - global_states.CURRENT_Y
        
        heading_x = 1 if delta_x > 0 else 3  
        heading_y = 2 if delta_y > 0 else 0  
        
        if delta_y != 0 and global_states.CURRENT_HEADING == heading_y:
            steps_to_take = min(MAX_STRIDE, abs(delta_y))
            print(f"Path clear. Leaping {steps_to_take} steps along Y-axis...")
            forward(steps_to_take)
            update_grid_position(steps=steps_to_take, is_moving_forward=True)
            
        elif delta_x != 0 and global_states.CURRENT_HEADING == heading_x:
            steps_to_take = min(MAX_STRIDE, abs(delta_x))
            print(f"Path clear. Leaping {steps_to_take} steps along X-axis...")
            forward(steps_to_take)
            update_grid_position(steps=steps_to_take, is_moving_forward=True)
            
        else:
            
            if delta_y != 0:
                face_heading(heading_y)
                steps_to_take = min(MAX_STRIDE, abs(delta_y))
            else:
                face_heading(heading_x)
                steps_to_take = min(MAX_STRIDE, abs(delta_x))
                
            print(f"Path clear. Turned, now leaping {steps_to_take} steps...")
            forward(steps_to_take)
            update_grid_position(steps=steps_to_take, is_moving_forward=True)
            
        print(f"Current Position: ({global_states.CURRENT_X}, {global_states.CURRENT_Y})")
        
    print(f"Arrived safely at destination: ({target_x}, {target_y})")
    ares.flip_LED()
    time.sleep(2)
    ares.flip_LED()
    

def go_to_grid_point(target_x, target_y):
    """
    Navigates to a grid coordinate
    """
    
    current_x = global_states.CURRENT_CELL[0]
    current_y = global_states.CURRENT_CELL[1]
    
    print(f"--- Navigating from ({current_x}, {current_y}) to ({target_x}, {target_y}) ---")
    
    delta_x = target_x - current_x
    delta_y = target_y - current_y
    
    # Determine headings
    heading_x = 1 if delta_x > 0 else 3  # 1 is East (+X), 3 is West (-X)
    heading_y = 2 if delta_y > 0 else 0 # 0 is North (-Y), 2 is South (+Y) 
    
    def move_along_x():
        if delta_x != 0:
            face_heading(heading_x)
            forward(abs(delta_x))
            current_x = target_x
            
    def move_along_y():
        if delta_y != 0:
            face_heading(heading_y)
            forward(abs(delta_y))
            current_y = target_y

    
    if delta_y != 0 and global_states.CURRENT_HEADING == heading_y:
        print("Moving along Y-axis first.")
        move_along_y()
        move_along_x()
    else:
        print("Moving along X-axis first.")
        move_along_x()
        move_along_y()
        
    print(f"Arrived successfully at destination: ({target_x}, {target_y})")
    
    global_states.CURRENT_CELL = [target_x, target_y]
    
    

def face_heading(target_heading):
    """
    0 = North (+Y), 1 = East (+X), 2 = South (-Y), 3 = West (-X)
    """
    current = global_states.CURRENT_HEADING
    
    if current == target_heading:
        return
        
    diff = (target_heading - current) % 4
    
    if diff == 1:
        print("Turning Right to face target heading...")
        right()
    elif diff == 2:
        print("Turning Around to face target heading...")
        turn_around()
    elif diff == 3:
        print("Turning Left to face target heading...")
        left()
        
    global_states.CURRENT_HEADING = target_heading
    
   

	
def turn_and_align(angle):
    """
    Executes an open-loop turn by a specified angle, followed by 
    a closed-loop visual PID correction to level the grid line.
    """

    print(f"Executing initial approximate turn ({angle} radians)...")
    ares.move_relative(0.0, angle) 
    
    # Time to settle
    if abs(angle) == math.pi:
        time.sleep(4) 
    else:
        time.sleep(2.5)
    
    
    Kp = 0.01   
    Ki = 0.000
    Kd = 0.001  
    PID_TIMEOUT_SECONDS = 5.0 

    ERROR_TOLERANCE = 0.0     # Acceptable error
    CONSECUTIVE_FRAMES = 10    

    integral = 0.0
    prev_error = 0.0
    prev_time = time.time()
    pid_start_time = time.time()
    aligned_count = 0
    
    
    while True:
        if time.time() - pid_start_time > PID_TIMEOUT_SECONDS:
            print(f"Warning: PID timeout reached ({PID_TIMEOUT_SECONDS}s). Forcing exit.")
            ares.set_velocity(0.0, 0.0) 
            break
		
        frame_bytes_floor = ares.get_camera_frame("floor")
        
        if frame_bytes_floor:
            arr = np.frombuffer(frame_bytes_floor, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
        
            if img is not None:
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
                
                lower_white = np.array([0, 0, 200])
                upper_white = np.array([180, 30, 255])
    
                white_mask = cv2.inRange(hsv_roi, lower_white, upper_white)
                
                
                # Horizontal Segment
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
                        cv2.circle(img, (global_cX, global_cY), 5, (0, 0, 255), -1)
                
                for i in range(len(line_points) - 1):
                    cv2.line(img, line_points[i], line_points[i+1], (0, 255, 0), 3)

                cv2.rectangle(img, (roi_start_x, roi_start_y), (roi_end_x, roi_end_y), (255, 255, 0), 2)
                
                # PID
                if len(line_points) >= 2:
                    left_y = line_points[0][1]
                    right_y = line_points[-1][1]
                    
                    # Tilt Error
                    error = left_y - right_y 
                    
                    current_time = time.time()
                    dt = current_time - prev_time if current_time - prev_time > 0 else 0.01
                    
                    integral += error * dt
                    derivative = (error - prev_error) / dt
                    
                    angular_vel = (Kp * error) + (Ki * integral) + (Kd * derivative)
                    
                    ares.set_velocity(0.0, angular_vel)
                    
                    
                    if abs(error) <= ERROR_TOLERANCE:
                        aligned_count += 1
                        if aligned_count >= CONSECUTIVE_FRAMES:
                            print("Turn and alignment complete! Line is level.")
                            ares.set_velocity(0.0, 0.0)
                            break
                    else:
                        aligned_count = 0 
                        
                    prev_error = error
                    prev_time = current_time
                    
                    cv2.line(img, (0, left_y), (width, left_y), (255, 0, 0), 1)

                else:
                    ares.set_velocity(0.0, 0.0)

                cv2.imshow("Grid Alignment Vision", img)
                cv2.imshow("ROI White Mask", white_mask) 
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    print("Stream stopped by user.")
                    ares.set_velocity(0.0, 0.0)
                    break
            else:
                print("Error: Could not decode camera frame.")
        else:
            print("Failed to get frames. API might be down.")
            time.sleep(1)

    cv2.destroyAllWindows()
    
def move_and_align(cells, direction_multiplier):
    """
    Executes an open-loop linear move, actively tracks the lines crossed, 
    and finishes with a closed-loop PID alignment to the center.
    """
    
    # PID
    Kp = 0.01  
    Ki = 0.000  
    Kd = 0.001  
    PID_TIMEOUT_SECONDS = 5.0 

    ERROR_TOLERANCE = 1.0      # Acceptable difference in pixels for linear alignment
    CONSECUTIVE_FRAMES = 10   
    ESTIMATED_TIME_PER_CELL_COUNT = [0, 3.2, 3.7, 4.5, 5.2, 5.4, 5.6, 5.8, 6.1, 6.5, 6.8, 7.5, 8.2, 9.0, 9.7, 10.7, 11.5, 12.4, 13.0, 13.7, 14.2, 14.4, 14.6, 14.3, 14.5]
    
    
    distance = cells * global_states.CELL_SIZE * direction_multiplier
	
    print(f"Executing approximate move: {distance} meters...")
    
    ares.move_relative(distance, 0.0)
    
    # tracking
    print("Tracking lines during movement...")
    cell_count = 0
    prev_avg_y = None
    
    # Rough Time to finish
    settle_time_limit = time.time() + ESTIMATED_TIME_PER_CELL_COUNT[cells]
    
    while time.time() < settle_time_limit:
        frame_bytes_floor = ares.get_camera_frame("floor")
        
        if frame_bytes_floor:
            arr = np.frombuffer(frame_bytes_floor, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
            
            if img is not None:
                current_y, processed_img, white_mask = cv.detect_line_y(img)
                height, width = processed_img.shape[:2]
                
                THRESHOLD_Y = global_states.MOVE_RELATIVE_REFERENCE_Y
                
                if current_y is not None:
                    if prev_avg_y is not None:
                        
                        roi_height = int(height * (1 - global_states.TOP_CROP_RATIO * 2))
                        jump_threshold = roi_height * 0.4 
                        is_jump = abs(current_y - prev_avg_y) > jump_threshold
                        
                        if not is_jump:
                            if direction_multiplier > 0 and prev_avg_y <= THRESHOLD_Y and current_y > THRESHOLD_Y:
                                cell_count += 1
                                print(f"[+] Grid Crossed Forward! Tally: {cell_count}/{cells}")
                            
                            elif direction_multiplier < 0 and prev_avg_y >= THRESHOLD_Y and current_y < THRESHOLD_Y:
                                cell_count += 1
                                print(f"[-] Grid Crossed Backward! Tally: {cell_count}/{cells}")
                                
                    prev_avg_y = current_y
                else:
                    prev_avg_y = None 

                # Draw
                cv2.line(processed_img, (0, int(THRESHOLD_Y)), (width, int(THRESHOLD_Y)), (0, 255, 255), 2)
                cv2.putText(processed_img, f"Tally: {cell_count}/{cells}", (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 2)
                
                cv2.imshow("Grid Alignment Vision", processed_img)
                cv2.imshow("ROI White Mask", white_mask) 
                cv2.waitKey(1)
                
    print(f"Open-loop move settled. Final tally: {cell_count} out of {cells} grids detected.")
    
    # Discrete PID accounting for possible network lag
    print("Starting discrete visual correction to center on GRID_CENTER_REFERENCE_Y...")
    
    
    ares.set_velocity(0.0, 0.0)
    time.sleep(0.5) 
    
    Kp_discrete = 0.006
    MAX_NUDGE_SPEED = 0.55 
    
    pid_start_time = time.time()
    aligned_count = 0
    
    while True:
        
        if time.time() - pid_start_time > PID_TIMEOUT_SECONDS:
            print(f"Warning: Discrete timeout reached ({PID_TIMEOUT_SECONDS}s). Forcing exit.")
            ares.set_velocity(0.0, 0.0) 
            break
        
        
        frame_bytes_floor = ares.get_camera_frame("floor")
        
        if frame_bytes_floor:
            arr = np.frombuffer(frame_bytes_floor, dtype=np.uint8)
            img = cv2.imdecode(arr, cv2.IMREAD_COLOR)
        
            if img is not None:
                current_y, processed_img, white_mask = cv.detect_line_y(img)
                
                if current_y is not None:
                    
                    error = global_states.GRID_CENTER_REFERENCE_Y - current_y 
                    
                    if abs(error) <= ERROR_TOLERANCE:
                        aligned_count += 1
                        
                        if aligned_count >= 2: 
                            print("Linear Alignment complete! Robot is perfectly centered.")
                            ares.set_velocity(0.0, 0.0) 
                            break
                    else:
                        aligned_count = 0 
                        
                        nudge_velocity = error * Kp_discrete
                        
                        nudge_velocity = max(min(nudge_velocity, MAX_NUDGE_SPEED), -MAX_NUDGE_SPEED)
                        
                        ares.set_velocity(nudge_velocity, 0.0)
                        time.sleep(0.1) 
                        
                        ares.set_velocity(0.0, 0.0)
                        time.sleep(0.2) 
                    
                    
                    cv2.line(processed_img, (0, int(global_states.GRID_CENTER_REFERENCE_Y)), 
                             (processed_img.shape[1], int(global_states.GRID_CENTER_REFERENCE_Y)), (255, 0, 255), 2)

                else:
                    ares.set_velocity(0.0, 0.0) 

                cv2.imshow("Grid Alignment Vision", processed_img)
                cv2.imshow("ROI White Mask", white_mask) 
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    print("Stream stopped by user.")
                    ares.set_velocity(0.0, 0.0)
                    break
    
    
def left():
    print("Left Turn...")
    turn_and_align(math.pi / 2)


def right():
	print("Right Turn...")
	turn_and_align(-math.pi / 2)


def turn_around():
    print("180 Degree Turn...")
    turn_and_align(math.pi)
    
def forward(cells):
    print(f"Initiating Move Forward ({cells} grids)...")
    move_and_align(cells, direction_multiplier=1.0)


def backward(cells):
    print(f"Initiating Move Backward ({cells} grids)...")
    move_and_align(cells, direction_multiplier=-1.0)
