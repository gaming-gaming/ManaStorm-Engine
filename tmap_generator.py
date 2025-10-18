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

# Test meshes
meshes = [
    {
        'name': 'Cube',
        'vertices': [(0,0,0),(1,0,0),(1,1,0),(0,1,0)],
        'normals': [(0,0,1)]*4,
        'uvs': [(0,0),(1,0),(1,1),(0,1)],
        'material': 'Default'
    },
    {
        'name': 'Pyramid',
        'vertices': [(0,0,0),(1,0,0),(0.5,0,1),(0.5,1,0.5)],
        'normals': [(0,1,0)]*4,
        'uvs': [(0,0),(1,0),(0.5,1),(0.5,0.5)],
        'material': 'Stone'
    }
]

spawn = (0.5, 0.0, 0.5)
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