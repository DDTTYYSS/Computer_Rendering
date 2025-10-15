import bpy, json, mathutils
import os, bpy; p=bpy.path.abspath("//"); print(p, os.access(p, os.W_OK))
import tempfile

def world_coords_of_plane_corners(obj):
    # assumes a plane mesh with 4 verts; returns them in world space
    deps = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(deps)
    mesh = eval_obj.to_mesh()
    corners = []
    for v in mesh.vertices[:4]:
        corners.append((obj.matrix_world @ v.co).to_tuple())
    eval_obj.to_mesh_clear()
    return corners

def gaze_from_camera(cam_obj):
    # Blender camera looks along -Z in local space; world dir is -Z transformed
    return (cam_obj.matrix_world.to_3x3() @ mathutils.Vector((0,0,-1))).normalized().to_tuple()

def is_sphere(obj):
    return obj.type == 'MESH' and ('sphere' in obj.name.lower() or 'uvsphere' in obj.data.name.lower())

def is_cube(obj):
    return obj.type == 'MESH' and ('cube' in obj.name.lower() or 'cube' in obj.data.name.lower())

def is_plane(obj):
    return obj.type == 'MESH' and ('plane' in obj.name.lower() or 'plane' in obj.data.name.lower())

scene_data = []
for obj in bpy.context.scene.objects:
    entry = {
        "name": obj.name,
        "type": obj.type,
        "location": list(obj.location),
        "rotation_euler_radians": list(obj.rotation_euler),
        "scale": list(obj.scale),
    }

    if obj.type == 'CAMERA':
        cam = obj.data
        entry["camera"] = {
            "focal_length_mm": cam.lens,
            "sensor_width_mm": cam.sensor_width,
            "sensor_height_mm": cam.sensor_height,
            "resolution_px": [bpy.context.scene.render.resolution_x,
                              bpy.context.scene.render.resolution_y],
            "gaze_dir_ws": list(gaze_from_camera(obj))
        }

    if obj.type == 'LIGHT' and obj.data.type == 'POINT':
        entry["light"] = {
            "kind": "POINT",
            "radiant_intensity": obj.data.energy
        }

    if is_sphere(obj):
        # Treat radius as average of world scales (assumes unit sphere mesh)
        avg_scale = sum(obj.scale) / 3.0
        entry["sphere"] = {"radius": avg_scale}

    if is_cube(obj):
        entry["cube"] = {"uniform_scale": (obj.scale[0] + obj.scale[1] + obj.scale[2]) / 3.0}

    if is_plane(obj):
        entry["plane"] = {"corners_ws": world_coords_of_plane_corners(obj)}

    scene_data.append(entry)
    
def _safe_out_path(filename: str) -> str:
    # Prefer the .blend directory
    blend_dir = bpy.path.abspath("//")
    if blend_dir and os.access(blend_dir, os.W_OK):
        return os.path.join(blend_dir, filename)
    # Fallback to a guaranteed-writable temp directory
    return os.path.join(tempfile.gettempdir(), filename)

out_path = _safe_out_path(f"{bpy.context.scene.name}_scene.json")
with open(out_path, "w") as f:
    json.dump(scene_data, f, indent=2)
print("✅ Wrote:", out_path)

