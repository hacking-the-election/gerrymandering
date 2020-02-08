import sys

import communities


if __name__ == "__main__":
    
    print(communities.create_communities(*sys.argv[1:]))