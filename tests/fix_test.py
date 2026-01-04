import re

with open('test_undirected_adjlist_basic.cpp', 'r') as f:
    content = f.read()

# Fix (*v).size() -> (*v).edges_size()
content = re.sub(r'\(\*v\)\.size\(\)', r'(*v).edges_size()', content)
content = re.sub(r'\(\*v\d+\)\.size\(\)', lambda m: m.group(0).replace('.size()', '.edges_size()'), content)  
content = re.sub(r'v\.size\(\)', r'v.edges_size()', content)

# Fix (*v).edges() calls - these need graph and key
# This is more complex, need to handle context

# Fix (*v).begin() -> (*v).edges_begin(g, key)
# Also complex

print("Basic replacements done. Manual fixes still needed for .edges() and .begin() calls.")
with open('test_undirected_adjlist_basic.cpp', 'w') as f:
    f.write(content)
