import requests
from global_states import API_BASE


def post_apriltag(raw, order, x, y):
    try:
        r = requests.post(f"{API_BASE}/april_tag", json={"raw": raw, "order" : order, "x" : x, "y" : y}, timeout=2)
        if r.status_code == 200:
            return True
    except requests.RequestException as e:
        print(f"Failed to send april_tag: {e}")
    return False
    

def flip_LED():
    try:
        r = requests.get(f"{API_BASE}/led", timeout=2)
        if r.status_code == 200:
            data = r.json()
            if data['led']:
                try:
                    r = requests.post(f"{API_BASE}/led", json={"state": 0}, timeout=1)
                    return True
                except requests.RequestException:
                    return False
            else:
                try:
                    r = requests.post(f"{API_BASE}/led", json={"state": 1}, timeout=1)
                    return True
                except requests.RequestException:
                    return False
                
    except requests.RequestException as e:
        print(f"Failed to find LED: {e}")
    return None
    
def set_LED(mode , color):
    try:
        r = requests.post(f"{API_BASE}/utility/set_led", json={"state": mode, "color" : color}, timeout=2)
        if r.status_code == 200:
            return True
    except requests.RequestException as e:
        print(f"Failed to set LED: {e}")
    return False
    

def get_status():
    """Fetches the status of the robot."""
    try:
        r = requests.get(f"{API_BASE}", timeout=2)
        if r.status_code == 200:
            return r.json()
    except requests.RequestException as e:
        print(f"Failed to connect to simulation: {e}")
    return None
    
def check_api():
    try:
        r = requests.get(f"{API_BASE}/health", timeout=2)
        if r.status_code == 200:
            data = r.json()
            print("API ready.")
            if data.get("odom_available"):
                print("Odometry Available")
                if all(data.get("cameras").values()):
                    print("Cameras Available")
                else:
                    return False
            else:
                return False
            return True
    except requests.RequestException as e:
        print("API not reachable:", e)
    return False
    
def get_arena_metadata():
    try:
        r = requests.get(f"{API_BASE}/arena/metadata", timeout=2)
        if r.status_code == 200:
            return r.json()
    except requests.RequestException:
        pass
    return None


def get_odometry():
    """Fetches the current position and orientation of Ares."""
    try:
        r = requests.get(f"{API_BASE}/odometry", timeout=2)
        if r.status_code == 200:
            return r.json()
    except requests.RequestException as e:
        print(f"Failed to connect to simulation: {e}")
    return None
    
def get_imu():
    try:
        r = requests.get(f"{API_BASE}/imu", timeout=2)
        print("Status code:", r.status_code)
        print("Raw response:", r.text)

        if r.status_code == 200:
            data = r.json()
            print("Parsed JSON:", data)

            ang_vel = data["angular_velocity"]
            li_acc = data["linear_acceleration"]
            return ang_vel, li_acc
        else:
            print("HTTP request failed")

    except Exception as e:
        print("Exception occurred:", type(e)._name_, "-", e)

    return None

def set_velocity(velocity, omega):
    """Sends a movement command to Ares. 
       velocity (m/s) -> positive is forward
       omega (rad/s) -> positive is counter-clockwise"""
    try:
        r = requests.post(
            f"{API_BASE}/set_velocity", 
            json={"velocity": velocity, "omega": omega}, 
            timeout=1
        )
        return r.status_code == 200
    except requests.RequestException:
        return False
        
def move_relative(distance, rotation):
    """distance: meters (positive=forward), rotation: radians (positive=CCW)"""
    try:
        r = requests.post(f"{API_BASE}/move_relative", json={"distance": distance, "rotation": rotation}, timeout=1)
        return r.status_code == 200
    except requests.RequestException:
        return False
        
def stop():
    """Emergency Stop"""
    try:
        requests.post(f"{API_BASE}/stop", json={}, timeout=1)
    except requests.RequestException:
        pass
        
def get_camera_frame(cam_id):
    """cam_id: 'front_left', 'front_right', or 'floor'"""
    try:
        r = requests.get(f"{API_BASE}/camera/{cam_id}/frame", timeout=2)
        if r.status_code == 200:
            return r.content  # raw JPEG bytes
    except requests.RequestException:
        pass
    return None
    


