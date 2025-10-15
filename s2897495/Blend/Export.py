import bpy, json, mathutils, os, tempfile

def world_coords_of_plane_corners(obj):
    deps = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(deps)
    mesh = eval_obj.to_mesh()
    mw = eval_obj.matrix_world
    corners = []
    for v in mesh.vertices[:4]:
        corners.append((mw @ v.co).to_tuple())
    eval_obj.to_mesh_clear()
    return corners

def basis_from_matrix_world(obj):
    # Blender camera local axes: right(+X), up(+Y), forward(-Z)
    R = obj.matrix_world.to_3x3()
    right_ws   = (R @ mathutils.Vector(( 1, 0,  0))).normalized().to_tuple()
    up_ws      = (R @ mathutils.Vector(( 0, 1,  0))).normalized().to_tuple()
    forward_ws = (R @ mathutils.Vector(( 0, 0, -1))).normalized().to_tuple()
    return right_ws, up_ws, forward_ws

def is_sphere(obj):
    return obj.type == 'MESH' and ('sphere' in obj.name.lower() or 'sphere' in obj.data.name.lower())

def is_cube(obj):
    return obj.type == 'MESH' and ('cube' in obj.name.lower() or 'cube' in obj.data.name.lower())

def is_plane(obj):
    return obj.type == 'MESH' and ('plane' in obj.name.lower() or 'plane' in obj.data.name.lower())

scene = bpy.context.scene
res_x = int(round(scene.render.resolution_x * scene.render.resolution_percentage / 100))
res_y = int(round(scene.render.resolution_y * scene.render.resolution_percentage / 100))

scene_data = []
for obj in scene.objects:
    # Skip unsupported types (optional)
    if obj.type not in {'CAMERA', 'LIGHT', 'MESH'}:
        continue

    entry = {
        "name": obj.name,
        "type": obj.type,
        "location": list(obj.location),
        "rotation_euler_radians": list(obj.rotation_euler),
        "scale": list(obj.scale),
    }

    if obj.type == 'CAMERA':
        cam = obj.data
        right_ws, up_ws, forward_ws = basis_from_matrix_world(obj)
        entry["camera"] = {
            "focal_length_mm": cam.lens,
            "sensor_width_mm": cam.sensor_width,
            "sensor_height_mm": cam.sensor_height,
            "resolution_px": [res_x, res_y],
            "gaze_dir_ws": list(forward_ws),
            "up_ws": list(up_ws),
            "right_ws": list(right_ws)
        }

    elif obj.type == 'LIGHT' and obj.data.type == 'POINT':
        entry["light"] = {
            "kind": "POINT",
            "radiant_intensity": obj.data.energy
        }

    elif is_sphere(obj):
        # Assume unit sphere source; export a 1D radius
        avg_scale = (obj.scale[0] + obj.scale[1] + obj.scale[2]) / 3.0
        entry["sphere"] = {"radius": avg_scale}

    elif is_cube(obj):
        entry["cube"] = {"uniform_scale": (obj.scale[0] + obj.scale[1] + obj.scale[2]) / 3.0}

    elif is_plane(obj):
        entry["plane"] = {"corners_ws": world_coords_of_plane_corners(obj)}

    else:
        # unhandled mesh like cylinder/meta -> skip
        continue

    scene_data.append(entry)

def _safe_out_path(filename: str) -> str:
    blend_dir = bpy.path.abspath("//")
    if blend_dir and os.access(blend_dir, os.W_OK):
        return os.path.join(blend_dir, filename)
    return os.path.join(tempfile.gettempdir(), filename)

out_path = _safe_out_path(f"../ASCII/{scene.name}_scene.json")
with open(out_path, "w") as f:
    json.dump(scene_data, f, indent=2)
print("✅ Wrote:", out_path)
