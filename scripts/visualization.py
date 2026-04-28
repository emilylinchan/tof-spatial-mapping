import serial
import time
import numpy as np
import open3d as o3d

# Configuration
SERIAL_PORT = "COM11"    
BAUD_RATE   = 115200
TIMEOUT_S   = 2
NUM_MEASUREMENTS = 64
FILE_NAME = "ToF_data.xyz" 

# Functions

def collect_scan(ser):
    """Read one full 360deg scan of NUM_MEASUREMENTS distance measurements in mm"""
    
    ser.reset_input_buffer()   # flush stale data before reading
    measurements = []
    
    while len(measurements) < NUM_MEASUREMENTS:
        line = ser.readline() # read until '\n'
        decoded = line.decode('utf-8').strip()
        if decoded.isdigit():
            measurements.append(int(decoded))
            
    return measurements


def convert_xyz(distances, x_coord, sensor_height_m=0.068): #0.068 = 6.8cm
    """Convert distance measurements to cartesian coordinates (sensor at origin, measurements taken in YZ plane)"""
   
    # Convert mm to meters
    distances = np.array(distances) / 1000.0  
    
    # Create an array of evenly spaced angles over [0, 2*pi]] (sensor starts at floor = -pi/2)
    angles = np.linspace(-np.pi/2, 3 * np.pi/2, NUM_MEASUREMENTS, endpoint=False)

    # Use SOH CAH TOA trigonometry for conversion
    x = [x_coord] * NUM_MEASUREMENTS # fixed x coordinate per scan (but increments as vehicle travels forward)
    y = (distances * np.sin(angles)) 
    z = (distances * np.cos(angles) + sensor_height_m)
    
    points = np.column_stack((x, y, z)) # 2D array of x, y, z coordinates for this scan
    
    return points     


def visualize_data(data_file):
    """Generate 3D lines and render the point cloud to visualize the data in the file"""
    
    # Read the scan data from .xyz file
    pcd = o3d.io.read_point_cloud(f"{data_file}", format="xyz") # Returns open3d.geometry.PointCloud object
    points = np.asarray(pcd.points)                             # Extract just the 3D coords from the object
    
    num_scans = len(points) // NUM_MEASUREMENTS
            
    # Give each vertex a unique number
    yz_slice_vertex = []
    for i in range(NUM_MEASUREMENTS*num_scans): 
        yz_slice_vertex.append(i)

    # Define coordinates to connect lines in each yz slice        
    lines = []  
    for scan in range(num_scans):
        base = scan * NUM_MEASUREMENTS
        for i in range(0, NUM_MEASUREMENTS):
            lines.append([yz_slice_vertex[base + i], yz_slice_vertex[base + (i + 1) % NUM_MEASUREMENTS]])

    # Define coordinates to connect lines between current and next yz slice        
    for scan in range(num_scans-1): 
        base = scan * NUM_MEASUREMENTS 
        # Connect point 'i' in the current slice to point 'i' in the next slice
        for i in range(NUM_MEASUREMENTS):
            lines.append([yz_slice_vertex[base + i], yz_slice_vertex[base + i + NUM_MEASUREMENTS]])
     
    # Map the lines to the 3D coordinate vertices
    line_set = o3d.geometry.LineSet(points=o3d.utility.Vector3dVector(np.asarray(pcd.points)),lines=o3d.utility.Vector2iVector(lines))

    # Visualize point cloud data with lines   
    o3d.visualization.draw_geometries([line_set])


if __name__ == "__main__":
    
    # STEP 1 - Collect Data
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT_S) 
    print("Serial port opened.")
    time.sleep(1)
    
    all_scans = [] 

    while True:
        # Obtain serial data
        line = ser.readline()
        decoded = line.decode('utf-8').strip()
        
        # Start flag from MCU
        if decoded == "START":
            scan = collect_scan(ser)
            all_scans.append(scan)
            print(f"Scan collected: {scan}")
            
        # End flag from MCU
        if decoded == "END":
            break
    
    ser.close()
    
    # STEP 2 - Process and store data in a file
    all_points = []
    x_coord = 0
    
    for scan in all_scans:
        all_points.append(convert_xyz(scan, x_coord))   # Convert distance measurements to cartesian coords
        x_coord += 0.28                                 # Vehicle moves forward every 28 cm

    # Write all points to a file using context manager
    with open(f"{FILE_NAME}", "w", newline='') as f:
        for scan_points in all_points:  # Each scan_point is a (NUM_MEASUREMENT x 3) array
            for row in scan_points:     # Each row is a single [x,y,z] point
                f.write(f"{row[0]} {row[1]} {row[2]}\n")
        
    print(f"Wrote {len(all_scans)} scans to {FILE_NAME}")
    
    # STEP 3 - Visualize data
    visualize_data(FILE_NAME)
