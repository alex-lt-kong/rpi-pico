module square_frame(groove_width, groove_length, groove_thickness, groove_height) {
    difference() {
        // Outer square
        cube([groove_width, groove_length, groove_height]);        
        // Inner square (cut out)
        translate([groove_thickness, groove_thickness, -1])  // Adjust for thickness
            cube([groove_width - 2 * groove_thickness, groove_length - 2 * groove_thickness, groove_height + 1]);
    }
}

// Example usage
square_frame(30, 100, 5, 20);  // Call the function with desired parameters