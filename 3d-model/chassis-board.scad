pico_w_width = 21;
pico_w_length = 51;
margin = 1;

side_panel_height = 30;
side_panel_thickness = 4;

height = 3;
width = pico_w_width + margin * 2 + side_panel_thickness * 2;
length = 100;

base = [
    [0, 0],
    [0, length],
    [width, length],
    [width, 0]
];

board_support_thickness = 2;
board_support = [
    [0, 0],
    [0, pico_w_length - 5],
    [width, pico_w_length - 5],
    [width, 0],
    [width - side_panel_thickness - board_support_thickness, 0],
    [width - side_panel_thickness - board_support_thickness, pico_w_length - board_support_thickness - 5],
    [side_panel_thickness + board_support_thickness, pico_w_length - board_support_thickness - 5],
    [side_panel_thickness + board_support_thickness, 0]
];


side_panel = [
    [0, 0],
    [0, length],    
    [side_panel_thickness + board_support_thickness, length],
    [side_panel_thickness, 0]
];

delta_length = 42;
delta_base = [
    [0, 0],
    [0, delta_length],
    [width / 2, delta_length],
    [width / 2, 0]
];


module hole_cylinder() {
    cylinder(h = side_panel_height, r = 2.1/2, center = true);
}
holes_x_offset = side_panel_thickness + (21 - 11.4) / 2;

difference() {
    union() {
        linear_extrude(height = height) {
            polygon(base);
        }
        linear_extrude(height = height * 2) {
            polygon(board_support);
        }
        // left side panel
        linear_extrude(height = side_panel_height) {
            polygon(side_panel);
        }
        // right side panel
        translate([width - side_panel_thickness, 0, 0])
        linear_extrude(height = side_panel_height) {
            polygon(side_panel);
        }
    }
    
    translate([holes_x_offset, 2, 0])
        hole_cylinder();
    translate([width - holes_x_offset, 2, 0])
        hole_cylinder();
    
    translate([holes_x_offset, 2 + 48.26, 0])
        hole_cylinder();
    translate([width - holes_x_offset, 2 + 48.26, 0])
        hole_cylinder();
    
    translate([holes_x_offset, 2 + 48.26 + 30, 0])
        hole_cylinder();
    translate([width - holes_x_offset, 2 + 48.26 + 30, 0])
        hole_cylinder();
}