import matplotlib.pyplot as plt
import csv
import tag_generator 

def main():
    print("Generating tag set... (This might take a moment)")
    
    # Call the function from your imported module
    tag_set = tag_generator.generate_unique_tag_set()

    # Make sure we actually got data back
    if not tag_set:
        print("Error: Could not generate a tag set.")
        return
    
    # 1. Export to CSV
    csv_filename = "Raspberry Pi\\April Tag\\Helper Functions\\generated_apriltags.csv"
    with open(csv_filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        # Write the header row
        writer.writerow(["AprilTag Number", "Order", "X Position", "Y Position"])
        
        # Write the data rows
        for item in tag_set:
            writer.writerow([item['tag'], item['order'], item['x'], item['y']])
            
    print(f"\nData successfully saved to {csv_filename}")

    # 2. Visualize with Matplotlib
    print("Opening grid visualization...")
    
    # Set up the figure size
    fig, ax = plt.subplots(figsize=(10, 10))

    # Loop through the tags to plot them and add text labels
    for item in tag_set:
        x, y = item['x'], item['y']
        tag_id = item['tag']
        order = item['order']
        
        # Plot the dot
        ax.scatter(x, y, color='#d62728', s=100, zorder=5)
        
        # Add the text label above the dot
        label_text = f"Tag {tag_id}\n(Ord: {order})"
        ax.annotate(label_text, (x, y), textcoords="offset points", xytext=(0, 10), 
                    ha='center', fontsize=9, bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="gray", alpha=0.8))

    # Configure the 25x25 grid properties
    ax.set_xticks(range(25))
    ax.set_yticks(range(25))
    ax.grid(True, linestyle='--', alpha=0.6)
    
    # Set the limits slightly past 0 and 24 so dots on the edge don't get cut off
    ax.set_xlim(-1, 25)
    ax.set_ylim(-1, 25)
    
    # Add titles and labels
    ax.set_title('Generated AprilTag Locations (25x25 Grid)', fontsize=14, pad=20)
    ax.set_xlabel('X Coordinate (0 - 24)', fontsize=12)
    ax.set_ylabel('Y Coordinate (0 - 24)', fontsize=12)

    # Invert the Y axis if you want (0,0) at the top-left instead of bottom-left
    # ax.invert_yaxis() 

    # Show the plot
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()