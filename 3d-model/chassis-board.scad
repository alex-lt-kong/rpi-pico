use <./side_groove.scad>;

pico_w_width = 21;
pico_w_length = 51;
margin = 1;

side_panel_height = 40;
side_panel_thickness = 3;

base_thickness = 3;
base_width = pico_w_width + margin * 2 + side_panel_thickness * 2;
base_length = 100;

base = [
    [0, 0],
    [0, base_length],
    [base_width, base_length],
    [base_width, 0]
];

board_support_thickness = 2;
board_support = [
    [0, 0],
    [0, pico_w_length - 5],
    [base_width, pico_w_length - 5],
    [base_width, 0],
    [base_width - side_panel_thickness - board_support_thickness, 0],
    [base_width - side_panel_thickness - board_support_thickness, pico_w_length - board_support_thickness - 5],
    [side_panel_thickness + board_support_thickness, pico_w_length - board_support_thickness - 5],
    [side_panel_thickness + board_support_thickness, 0]
];


side_panel = [
    [0, 0],
    [0, base_length],    
    [side_panel_thickness + 0, base_length],
    [side_panel_thickness + 0, 0]
];

delta_length = 42;
delta_base = [
    [0, 0],
    [0, delta_length],
    [base_width / 2, delta_length],
    [base_width / 2, 0]
];


hole_radius = 3 / 2;
module hole_cylinder() {
    cylinder(h = side_panel_height, r = hole_radius, center = true);
}
holes_x_offset = (21 - 11.4) / 2;

module cable_groove() {
    groove_depth = 2;
    groove_length = base_width;
    groove_width = hole_radius * 2;
    linear_extrude(height = groove_depth) {
        polygon([
            [0, 0],
            [0, groove_width],    
            [groove_length, groove_width],
            [groove_length, 0]
        ]);
    }
}



module sensor_support() {
    width = 2;
    length = 20;
    height = base_thickness + 7;
    translate([(base_width - width) / 2, base_length * 5 / 7, 0])
    linear_extrude(height = height) {
        polygon([
            [0, 0],
            [0, length],    
            [width, length],
            [width, 0]
        ]);
    }
}

difference() {
    union() {
        color("green",0.5)
            linear_extrude(height = base_thickness) {
                polygon(base);
            }
        linear_extrude(height = base_thickness + 7) {
            polygon(board_support);
        }
        
        // left side panel
        color("red",0.5)        
        translate([0 - side_panel_thickness, 0, 0])
        linear_extrude(height = side_panel_height) {
            polygon(side_panel);
        }
        // right side panel
        color("red",0.5)
        translate([base_width, 0, 0])
        linear_extrude(height = side_panel_height) {
            polygon(side_panel);
        }        
        
        sensor_support();
    }
    
    row1_y_offset = 3;    
    translate([holes_x_offset, row1_y_offset, 0])
        hole_cylinder();
    translate([base_width - holes_x_offset, row1_y_offset, 0])
        hole_cylinder();
    translate([0, row1_y_offset - hole_radius, 0])
        cable_groove();
    
    row2_y_offset = 2 + 48.26;
    translate([holes_x_offset, row2_y_offset, 0])
        hole_cylinder();
    translate([base_width - holes_x_offset, row2_y_offset, 0])
        hole_cylinder();
    translate([0, row2_y_offset - hole_radius, 0])
        cable_groove();
    
    row3_y_offset = 2 + 48.26 + 30; 
    translate([holes_x_offset, row3_y_offset, 0])
        hole_cylinder();
    translate([base_width - holes_x_offset, row3_y_offset, 0])
        hole_cylinder();
    translate([0, row3_y_offset - hole_radius, 0])
        cable_groove();
    
    color("red",0.5)       
    translate([-1, 0, side_panel_height - 5])
        square_frame(base_width + 2, base_length - 2, 3, 3); 
}