pico_w_width = 21;
pico_w_length = 51;
margin = 1;

side_panel_height = 40;
side_panel_thickness = 2;

base_thickness = 4;
base_width = pico_w_width + margin * 2 + side_panel_thickness * 2;
base_length = 100;

module square_frame(groove_width, groove_length, groove_height) {
        cube([groove_width, groove_length, groove_height]);
}

// Example usage
square_frame(base_width + 2, base_length - 5, 2.8);  // Call the function with desired parameters