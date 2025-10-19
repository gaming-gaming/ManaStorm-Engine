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
        'name': 'Cube',
        'vertices': cube_vertices,
        'normals': cube_normals,
        'uvs': cube_uvs,
        'material': 'Default'
    },
    {
        'name': 'Floor',
        'vertices': floor_vertices,
        'normals': floor_normals,
        'uvs': floor_uvs,
        'material': 'Ground'
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
        write_mesh(f, mesh['name'], mesh['vertices'], mesh['normals'], mesh['uvs'], mesh['material'])
    
    # Spawn position
    f.write(struct.pack('<fff', *spawn))
    
    # Spawn rotation
    f.write(struct.pack('<fff', *rotation))
    
    # Map offset
    f.write(struct.pack('<fff', *offset))

print('Test TMAP file generated as test.tmap')
print(f'  - Cube: {len(cube_vertices)} vertices')
print(f'  - Floor: {len(floor_vertices)} vertices')