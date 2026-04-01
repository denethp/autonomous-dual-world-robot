import cv2
from apriltag import apriltag
from picamera2 import Picamera2
from apriltag_to_coordinates import decrypt_apriltag

if __name__ == "__main__":

    # Initialize the dictionary with keys 1 to 14, defaulting to None
    tag_coordinates = {i: None for i in range(1, 15)}

    # Initialize and start the Pi Camera
    picam2 = Picamera2()
    config = picam2.create_preview_configuration(main={"size": (640, 480)})
    picam2.configure(config)
    picam2.start()

    # Initialize the custom AprilTag detector
    detector = apriltag("tagStandard52h13")

    print("Camera started. Looking for tagStandard52h13...")
    print("Press 'q' in the video window to quit.")

    try:
        while True:
            # Grab the frame
            frame = picam2.capture_array()
            
            # Convert RGB to Grayscale for detection
            gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
            
            # Detect tags
            detections = detector.detect(gray)
            
            # Process detected IDs
            for detection in detections:
                tag_id = detection['id']
                print(f"Detected Tag ID: {tag_id}")
                
                # Call your decryption function
                decrypted_data = decrypt_apriltag(tag_id)
                
                # Check if data was successfully returned
                if decrypted_data:
                    order = decrypted_data.get('order')
                    x_coord = decrypted_data.get('x')
                    y_coord = decrypted_data.get('y')
                    
                    # Verify the order is within our 1-14 range before updating
                    if order in tag_coordinates:
                        tag_coordinates[order] = {'x': x_coord, 'y': y_coord}
                        print(f"--> Updated Map: Position {order} is at ({x_coord}, {y_coord})")
                    else:
                        print(f"--> Ignored: Tag {tag_id} returned an out-of-bounds order: {order}")
                
            # Convert RGB to BGR for OpenCV's display window
            display_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
            cv2.imshow("Scanner", display_frame)
            
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    finally:
        # Print the final dictionary state upon exiting
        print("\nFinal Tag Coordinates Map:")
        for key, val in tag_coordinates.items():
            print(f"Order {key}: {val}")
            
        # Safely close the camera and windows
        picam2.stop()
        cv2.destroyAllWindows()
