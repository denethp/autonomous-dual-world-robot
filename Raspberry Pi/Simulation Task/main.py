import cv2
import threading
import time
from apriltag import apriltag
from picamera2 import Picamera2
from apriltag_to_coordinates import decrypt_apriltag
import ares_move_functions as ares_move


tag_coordinates = {i: None for i in range(1, 15)}
tag_lock = threading.Lock()
current_target_order = 1 


def movement_logic():
    """Thread that monitors the dictionary and moves the robot."""
    global current_target_order
    
    while current_target_order <= 14:
        target_data = None
        
        
        with tag_lock:
            if tag_coordinates[current_target_order] is not None:
                target_data = tag_coordinates[current_target_order]
        
        if target_data:
            
            ares_move.go_to_grid_point_chunked(target_data['x'], target_data['y'])
            current_target_order += 1
        else:
            
            time.sleep(0.5)

def vision_scanner():
    """The original scanning logic, now inside a function."""
    picam2 = Picamera2()
    config = picam2.create_preview_configuration(main={"size": (640, 480)})
    picam2.configure(config)
    picam2.start()
    detector = apriltag("tagStandard52h13")

    try:
        while True:
            frame = picam2.capture_array()
            gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
            detections = detector.detect(gray)
            
            for detection in detections:
                tag_id = detection['id']
                decrypted_data = decrypt_apriltag(tag_id)
                
                if decrypted_data:
                    order = decrypted_data.get('order')
                    
                    with tag_lock:
                        
                        if order in tag_coordinates and tag_coordinates[order] is None:
                            tag_coordinates[order] = {
                                'x': decrypted_data.get('x'), 
                                'y': decrypted_data.get('y')
                            }
                            print(f"[SCANNER] Found and saved Order {order}")

            
            display_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
            cv2.imshow("Scanner", display_frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
    finally:
        picam2.stop()
        cv2.destroyAllWindows()

if __name__ == "__main__":
    # Start the movement thread
    move_thread = threading.Thread(target=movement_logic, daemon=True)
    move_thread.start()

    # Run the vision scanner in the main thread
    vision_scanner()
