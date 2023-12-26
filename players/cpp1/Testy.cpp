#include <bits/stdc++.h>
using namespace std;

int manhattanDistance(int x1, int y1, int x2, int y2) {
  return abs(x1 - x2) + abs(y1 - y2);
} //manhattan vzdialenost
vector<vector<int>> manhattanBody(int n,int m, int x, int y, int vzdialenost) {
  vector<vector<int>> body;

  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      if (manhattanDistance(i, j, x, y) <= vzdialenost) {
        body.push_back({i, j});
      }
    }
  }

  return body;
}//vrati vsetky body v manhattan vzdialenosti a menej

int main() {
  int n,m, x, y, vzdialenost;
  n=10;
  m=5;
  y=3;
  x=3;
  vzdialenost=1;
  vector<vector<int>> hlavny(n,vector<int>(m,0));
    hlavny[y][x]=1;


  

  vector<vector<int>> body = manhattanBody(n,m, x, y, vzdialenost);
//   vector<vector<int>> body = find_all_points_in_manhattan_distance(x, y, vzdialenost);
  


  for (vector<int> bod : body) {
    cout << "(" << bod[0] << ", " << bod[1] << ")" << endl;
    hlavny[bod[1]][bod[0]]=2;
  }
  hlavny[y][x]=1;
  for(auto i:hlavny){
    for(auto j:i){
        cout<<j;
    }
    cout<<endl;
  }

  

  return 0;
}