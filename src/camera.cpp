#include "camera.h"

// TODO make camera zoom in where the cursor currently is
// TODO zoom in only in steps that are pixel-perfect

// TODO make camera part of the renderer and implement an orthographic camera controller

// TODO rewrite everything related to positions:
// - entity positions are 3d floats in world space with relative pivots
// - colliders are relative to the topleft of the entity and in pixels
// - tile positions are 3d floats, their center being at {0,0}, {16,16} etc.
// - camera is a rect with the topleft in pixel-worldspace, width & height equal to screen_width & screen_height
// - camera returns screen positions as v3f for some reason...
//
// - upload world positions to renderer and update the camera with a uniform
//   (probably add a renderer_update_camera(cam) function to the API)

v3f camera_screen_to_world(Camera& cam, const v3f& cam_pos)
{
    cam.scale   = (f32) cam.rect.w / SCREEN_WIDTH;
    cam.scale   = 1;
    v3f cam_p   = cam_pos; // NOTE copy because of const (we have no const versions of the overloaded operators)
    v3f scaled_cam_pos = cam_p / cam.scale;
    v3f cam_size       = {{(f32) cam.rect.left, (f32) cam.rect.top, 0}};

    return scaled_cam_pos + cam_size;
}

v3f camera_world_to_screen(Camera& cam, const v3f& world_pos)
{
    cam.scale    = (f32) cam.rect.w / SCREEN_WIDTH;
    v3f cam_size = {(f32) cam.rect.left, (f32) cam.rect.top, 0};
    v3f world    = world_pos; // NOTE copy because of const (we have no const versions of the overloaded operators)

    return (world - cam_size) * cam.scale;
}

// TODO not working
void camera_zoom(Camera& cam, f32 zoom_factor, v2i zoom_pos)
{
    cam.rect.w *= zoom_factor;
    cam.rect.h *= zoom_factor;

    // calculate offset
    v2i new_center = {(i32) (cam.rect.w/2), (i32) (cam.rect.h/2)};
    //v3f new_center_f = camera_screen_to_world(cam, {(f32)new_center.x, (f32)new_center.y, 0});
    //new_center = {(i32) new_center_f.x, (i32) new_center_f.y};
    v2i offset     = new_center - zoom_pos;

    cam.rect.left += offset.x;
    cam.rect.top  += offset.y;
}
