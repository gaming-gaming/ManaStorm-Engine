import struct

def write_string(f, s):
    encoded = s.encode('utf-8')
    f.write(struct.pack('<H', len(encoded)))  # uint16 for string length
    f.write(encoded)

def write_mesh(f, name, vertices, normals, uvs, material):
    write_string(f, name)
    
    # Vertices
    f.write(struct.pack('<I', len(vertices)))
    for v in vertices:
        f.write(struct.pack('<fff', *v))
    
    # Normals
    f.write(struct.pack('<I', len(normals)))
    for n in normals:
        f.write(struct.pack('<fff', *n))
    
    # UVs
    f.write(struct.pack('<I', len(uvs)))
    for uv in uvs:
        f.write(struct.pack('<ff', *uv))
    
    # Material
    write_string(f, material)

# Create a proper cube with triangulated faces
# Each face needs 2 triangles (6 vertices per face, 36 total)
cube_vertices = [
    # Front face (2 triangles)
    (-1, -1, 1), (1, -1, 1), (1, 1, 1),
    (-1, -1, 1), (1, 1, 1), (-1, 1, 1),
    
    # Back face
    (1, -1, -1), (-1, -1, -1), (-1, 1, -1),
    (1, -1, -1), (-1, 1, -1), (1, 1, -1),
    
    # Left face
    (-1, -1, -1), (-1, -1, 1), (-1, 1, 1),
    (-1, -1, -1), (-1, 1, 1), (-1, 1, -1),
    
    # Right face
    (1, -1, 1), (1, -1, -1), (1, 1, -1),
    (1, -1, 1), (1, 1, -1), (1, 1, 1),
    
    # Top face
    (-1, 1, 1), (1, 1, 1), (1, 1, -1),
    (-1, 1, 1), (1, 1, -1), (-1, 1, -1),
    
    # Bottom face
    (-1, -1, -1), (1, -1, -1), (1, -1, 1),
    (-1, -1, -1), (1, -1, 1), (-1, -1, 1)
]

cube_normals = [
    (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1), (0, 0, 1),  # Front
    (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1), (0, 0, -1),  # Back
    (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0), (-1, 0, 0),  # Left
    (1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0), (1, 0, 0),  # Right
    (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0), (0, 1, 0),  # Top
    (0, -1, 0), (0, -1, 0), (0, -1, 0), (0, -1, 0), (0, -1, 0), (0, -1, 0)  # Bottom
]

cube_uvs = [(0, 0), (1, 0), (1, 1), (0, 0), (1, 1), (0, 1)] * 6

# Create a simple floor plane
floor_vertices = [
    (-10, -2, -10), (10, -2, -10), (10, -2, 10),
    (-10, -2, -10), (10, -2, 10), (-10, -2, 10)
]

floor_normals = [(0, 1, 0)] * 6
floor_uvs = [(0, 0), (10, 0), (10, 10), (0, 0), (10, 10), (0, 10)]

# Test meshes
meshes = [
    {
        'name': 'Cube_0',
        'vertices': cube_vertices,
        'offset': (0, 0, 0),
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'test_orange_wall'
    },
    {
        'name': 'Cube_1',
        'vertices': cube_vertices,
        'offset': (4, -1, 0),
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'test_orange_wall'
    },
    {
        'name': 'Cube_2',
        'vertices': cube_vertices,
        'offset': (6, -2, 0),
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'test_orange_wall'
    },
    {
        'name': 'Cube_3',
        'vertices': cube_vertices,
        'offset': (-6, 1, 0),
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'test_orange_wall'
    },
    {
        'name': 'Cube_4',
        'vertices': cube_vertices,
        'offset': (-8, 2, 2),
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'test_orange_wall'
    },
    {
        'name': 'Floor',
        'vertices': floor_vertices,
        'normals': floor_normals,
        'uvs': floor_uvs,
        'material': 'test_black_floor'
    }
]

spawn = (0.0, 0.0, 5.0)
rotation = (0.0, 0.0, 0.0)
offset = (0.0, 0.0, 0.0)

with open('test.tmap', 'wb') as f:
    # Magic bytes
    f.write(b'TMAP')
    
    # Version
    f.write(struct.pack('<I', 1))
    
    # Number of meshes
    f.write(struct.pack('<I', len(meshes)))
    
    # Write meshes
    for mesh in meshes:
        # Apply offset to vertices if present
        if 'offset' in mesh:
            offset_vec = mesh['offset']
            adjusted_vertices = [(v[0] + offset_vec[0], v[1] + offset_vec[1], v[2] + offset_vec[2]) for v in mesh['vertices']]
        write_mesh(f, mesh['name'], adjusted_vertices if 'offset' in mesh else mesh['vertices'], mesh['normals'], mesh['uvs'], mesh['material'])

    # Spawn position
    f.write(struct.pack('<fff', *spawn))
    
    # Spawn rotation
    f.write(struct.pack('<fff', *rotation))
    
    # Map offset
    f.write(struct.pack('<fff', *offset))

print('Test TMAP file generated as test.tmap')
print(f'  - Cube: {len(cube_vertices)} vertices')
print(f'  - Floor: {len(floor_vertices)} vertices')