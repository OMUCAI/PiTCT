import umsgpack
from pathlib import Path
from .config import Config, DES_FILE_EXTENSION

def _dijkstra(edges, num_node):
    """ 経路の表現
            [終点, 辺の値]
            A, B, C, D, ... → 0, 1, 2, ...とする """
    node = [float('inf')] * num_node    #スタート地点以外の値は∞で初期化
    node[0] = 0     #スタートは0で初期化

    node_name = [i for i in range(num_node)]     #ノードの名前を0~ノードの数で表す

    while len(node_name) > 0:
        r = node_name[0]

        #最もコストが小さい頂点を探す
        for i in node_name:
            if  node[i] < node[r]:
                r = i   #コストが小さい頂点が見つかると更新

        #最もコストが小さい頂点を取り出す
        min_point = node_name.pop(node_name.index(r))

        #経路の要素を各変数に格納することで，視覚的に見やすくする
        for factor in edges[min_point]:
            goal = factor[0]   #終点
            cost  = factor[1]   #コスト

            #更新条件
            if node[min_point] + cost < node[goal]:
                node[goal] = node[min_point] + cost     #更新

    return node


def min_distance(plant: str):
    conf = Config.get_instance()
    _path = Path(conf.SAVE_FOLDER / (plant + DES_FILE_EXTENSION))
    _byte = _path.read_bytes()
    _data = umsgpack.unpackb(_byte)

    states = _data["states"]

    edges = []
    marked_list = []

    # create edges
    for state in states:
        next_states = states[state]["next"]
        is_marked = states[state]["marked"]
        if is_marked:
            marked_list.append(state)
        
        if next_states is not None:
            cost = 1
            edge_list = [[next[1], cost] for next in next_states]
            edges.append(edge_list)
        else:
            edges.append([])

    opt_node = _dijkstra(edges=edges, num_node=len(edges))
    result = {}
    for goal in marked_list:
        result[goal] = opt_node[goal]
    return result


if __name__ == '__main__':
    Edges = [
        [[1, 1], [2, 1]],             # ← 頂点Aからの辺のリスト
        [[2, 1], [3, 1], [2, 1]],   # ← 頂点Bからの辺のリスト
        [[5, 2]],                       # ← 頂点Cからの辺のリスト
        [[4, 3]],                       # ← 頂点Dからの辺のリスト
        [[6, 2]],                       # ← 頂点Eからの辺のリスト
        [[4, 1], [6, 4]],             # ← 頂点Fからの辺のリスト
        []                                # ← 頂点Gからの辺のリスト
    ]

    #今の目的地の数は7つ（0~6: A~G）
    node_num = 7

    opt_node = _dijkstra(Edges, node_num)

    #以下は結果を整理するためのコード
    node_name = []
    for i in range(node_num):
        node_name.append(chr(ord('A') + i))    
    result = []
    for i in range(len(opt_node)):
        result.append(f"{node_name[i]} : {opt_node[i]}")
    print(f"'目的地:そこまでの最小コスト'\n\n{result}")