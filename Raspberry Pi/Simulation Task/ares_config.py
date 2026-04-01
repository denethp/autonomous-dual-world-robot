import time
import global_states
import ares_utils as ares
import ares_cv_functions as cv
import ares_move_functions as move


def config_ares():
	while not ares.check_api():
		print("An API point is down, retrying in 1s")
		time.sleep(1)
	
	arena_data = ares.get_arena_metadata()
	global_states.CELL_SIZE = arena_data["cell_size"] # in meters
	global_states.CURRENT_CELL = arena_data["start_cell"] # A List [X,Y]
	global_states.CURRENT_X = global_states.CURRENT_CELL[0]
	global_states.CURRENT_Y = global_states.CURRENT_CELL[1]
	print(global_states.CURRENT_CELL)
	
	cv.calibrate_reference_line()
	calibrate_initial_pose()
	
	
	
def calibrate_initial_pose():
    
    print("Calibrating initial position and orientation...")
    
    
    start_pose = ares.get_odometry()
    start_x = start_pose["pose"]["x"] 
    start_y = start_pose["pose"]["y"]
    
    move.forward(1) 
    
    time.sleep(1)
    end_pose = ares.get_odometry()
    end_x = end_pose["pose"]["x"] 
    end_y = end_pose["pose"]["y"] 
    
    
    delta_x = end_x - start_x
    delta_y = end_y - start_y
    
    if abs(delta_x) > abs(delta_y):
        if delta_x > 0:
            global_states.CURRENT_HEADING = 1  # East (+X)
            print("Initial Heading locked: EAST (+X)")
        else:
            global_states.CURRENT_HEADING = 3  # West (-X)
            print("Initial Heading locked: WEST (-X)")
    else:
        if delta_y > 0:
            global_states.CURRENT_HEADING = 2  # North (-Y)
            print("Initial Heading locked: NORTH (+Y)")
        else:
            global_states.CURRENT_HEADING = 0  # South (+Y)
            print("Initial Heading locked: SOUTH (-Y)")
            
    move.backward(1)


	
		

	
	


