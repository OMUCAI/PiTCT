import umsgpack
from pathlib import Path
import heapq
from typing import List, Tuple
from .config import Config, DES_FILE_EXTENSION

#https://qiita.com/Yuya-Shimizu/items/eefdc6f854534e90c988
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


def _dijkstra_with_route(edges, num_node, start_node, goal_node) -> Tuple[List[int], List]:
    """ 経路の表現
            [終点, 辺の値]
            A, B, C, D, ... → 0, 1, 2, ...とする """
    node = [float('inf')] * num_node    #スタート地点以外の値は∞で初期化
    node[start_node] = 0     #スタートは0で初期化
    
    node_name = []
    heapq.heappush(node_name, [0, [start_node]])

    while len(node_name) > 0:
        #ヒープから取り出し
        _, min_point = heapq.heappop(node_name)
        last = min_point[-1]
        if last == goal_node:
            return min_point, node  #道順とコストを出力させている

        #経路の要素を各変数に格納することで，視覚的に見やすくする
        for factor in edges[last]:
            goal = factor[0]   #終点
            cost = factor[1]   #コスト

            #更新条件
            if node[last] + cost < node[goal]:
                node[goal] = node[last] + cost     #更新
                #ヒープに登録
                heapq.heappush(node_name, [node[last] + cost, min_point + [goal]])

    return [], []

def _load_states(plant: str):
    conf = Config.get_instance()
    _path = Path(conf.SAVE_FOLDER / (plant + DES_FILE_EXTENSION))
    _byte = _path.read_bytes()
    _data = umsgpack.unpackb(_byte)

    states = _data["states"]
    return states


def _create_edge(states) -> list:
    edges = []
    for state in states:
        next_states = states[state]["next"]        
        if next_states is not None:
            cost = 1
            edge_list = [[next[1], cost] for next in next_states]
            edges.append(edge_list)
        else:
            edges.append([])
    return edges


def min_distance(plant: str):
    states = _load_states(plant)

    edges = _create_edge(states)
    marked_list = []

    # create marker list
    for state in states:
        is_marked = states[state]["marked"]
        if is_marked:
            marked_list.append(state)

    opt_node = _dijkstra(edges=edges, num_node=len(edges))
    result = {}
    for goal in marked_list:
        result[goal] = opt_node[goal]
    return result


def path_string_list(plant: str, start: int, goal: int) -> list:
    states = _load_states(plant)
    edges = _create_edge(states)

    path, _ = _dijkstra_with_route(edges=edges, num_node=len(edges), start_node=start, goal_node=goal)
    return path


# if __name__ == '__main__':
#     Edges = [
#         [[1, 4], [2, 3]],             # ← 頂点Aからの辺のリスト
#         [[2, 1], [3, 1], [4, 5]],   # ← 頂点Bからの辺のリスト
#         [[5, 2]],                       # ← 頂点Cからの辺のリスト
#         [[4, 3]],                       # ← 頂点Dからの辺のリスト
#         [[6, 2]],                       # ← 頂点Eからの辺のリスト
#         [[4, 1], [6, 4]],             # ← 頂点Fからの辺のリスト
#         []                                # ← 頂点Gからの辺のリスト
#     ]

    #今の目的地の数は7つ（0~6: A~G）
    # node_num = 7
    # goal = 6
    # opt_node = _dijkstra(Edges, node_num)

    # #以下は結果を整理するためのコード
    # node_name = []
    # for i in range(node_num):
    #     node_name.append(chr(ord('A') + i))    
    # result = []
    # for i in range(len(opt_node)):
    #     result.append(f"{node_name[i]} : {opt_node[i]}")
    # print(f"'目的地:そこまでの最小コスト'\n\n{result}")


    # opt_root, opt_cost = _dijkstra_with_route(Edges, node_num, 4, 5)    #道順とコストを出力させている
    # #出力を見やすく整理するための変換用辞書型リストの作成
    # root_converter = {}
    # cost_converter = {}
    # print(opt_root)
    # print(opt_cost)
    # for i in range(node_num):
    #     root_converter[i] = chr(ord('A') + i)
    #     cost_converter[i] = opt_cost[i]

    # arrow = " → "
    # result = ""
    
    # for i in range(len(opt_root)):
    #     if i > 0:
    #         result += arrow
    #     result += f"{root_converter[opt_root[i]]}({cost_converter[opt_root[i]]})"

    # print(f"ノード(そこまでのコスト)\n\n{result}")