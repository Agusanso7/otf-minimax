#pragma GCC optimize("Ofast")
#pragma comment(linker, "/STACK: 20000000")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")
#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <queue>
#include <random>
#include <set>
#include <vector>
#include <climits>
#include "list"
//#include <bits/stdc++.h>
#include <random>

using namespace std;

enum class GameState {
    MAXIMIZING_WON,
    MINIMIZING_WON,
    IN_PROGRESS
};

# define PASS_ACTION "PASS_ACTION"

class miniMaxTreeSearch {

public:

  map<string,int> visited_node_time; // todo: use hashes?
  vector<string> marked_actions;
  vector< vector<vector< pair<vector<string>, vector<string>> >> > automatas; // each automata is "from->to: <{action_specific_contr},{action_specific_non_contr}>"

  miniMaxTreeSearch(vector< vector<vector< pair<vector<string>, vector<string>> >> > &_automatas, vector<string> &_marked_actions) {
    automatas = _automatas; marked_actions = _marked_actions;
  }

  void solve() { // iterative deepening - todo: transposition tables
    for(int depth=0;depth<=15;depth++) {
      cerr<<"Analyzing depth "<<depth<<endl;
      vector<int> automata_indexes; for(auto&_:automatas) automata_indexes.push_back(0);
      int minimax_search = compute_minimax_search(automata_indexes, false, depth, INT_MIN, INT_MAX, false, 1);
      if(abs(minimax_search) == 1) {
        cerr<<"Minimizing (aka uncontrollable) player optimal score: "<<minimax_search<<endl;
        break;
      }
      cerr<<"Non conclusive result ..."<<endl;
    }
  }

  int compute_minimax_search(vector<int>& automata_indexes, bool marked, int depth, int alpha, int beta, bool isMaximizingPlayer, int time, bool uncontrollablePassed=false) {

    if(depth<0) return 0;

    GameState current_game_state = getGameState(automata_indexes, marked);
    if (current_game_state == GameState::MAXIMIZING_WON) return 1; // TODO play with "10 - depth" as score to favor faster wins or slower losses
    else if (current_game_state == GameState::MINIMIZING_WON) return -1;

    set<string> _actions = get_player_actions(automata_indexes, isMaximizingPlayer);
    vector<string> actions(_actions.begin(),_actions.end());

    string best_action = "NOTHING";
    random_shuffle(actions.begin(),actions.end());
    if (not isMaximizingPlayer) { // uncontrollable player - minimizing
      int best_score = INT_MAX;
      actions.push_back(PASS_ACTION); // uncontrollable player can also do nothing and pass
      random_shuffle(actions.begin(),actions.end());
      for(auto&action:actions) {
        // do action
        vector<int> next_automata_indexes = get_automata_indexes_from_action(automata_indexes, action, isMaximizingPlayer);
        bool next_marked = is_marked_action(action, marked_actions);
        uncontrollablePassed = action==PASS_ACTION;
        if(uncontrollablePassed) next_marked = marked; // not moving from node
        bool isMaximizingPlayerNextTurn = uncontrollablePassed ? true : false;
        assert(visited_node_time.count(node_to_string(automata_indexes, marked)) == 0);
        if(action != PASS_ACTION) {
          visited_node_time[node_to_string(automata_indexes, marked)] = time;
          if(marked) marked_visits.push_back(time);
        }
        // recursive call
        int child_score = compute_minimax_search(next_automata_indexes, next_marked, depth-1, alpha, beta, isMaximizingPlayerNextTurn, time+1, uncontrollablePassed);
        if(child_score<best_score) best_action = action;
        best_score = min(child_score, best_score); // move to worse best move from opponent (wants to maximize) | move to best move from me (I want to minimize)
        // undo action
        if(action != PASS_ACTION) {
          visited_node_time.erase(node_to_string(automata_indexes, marked));
          if(marked) marked_visits.pop_back();
        }

        if (best_score<=alpha) break;
        beta = min(beta, best_score);
      }
      //cerr<<"actions uncontrolled: "; for(auto&it:actions)cerr<<it<<"#"; cerr<<endl;
      //cerr<<"best score: "<<best_score<<", action: "<<best_action<<" for state "<<node_to_string(automata_indexes, marked)<<endl;
      return best_score;
    } else { // controllable player - maximizing
      if (actions.empty()) {
        return -1; // uncontrollable wins if controllable can't move -> won't see marked infinitely
      }
      int best_score = INT_MIN;
      for(auto&action:actions) {
        // do action
        vector<int> next_automata_indexes = get_automata_indexes_from_action(automata_indexes, action, isMaximizingPlayer);
        bool next_marked = is_marked_action(action, marked_actions);
        assert(visited_node_time.count(node_to_string(automata_indexes, marked)) == 0);
        visited_node_time[node_to_string(automata_indexes, marked)] = time;
        if(marked) marked_visits.push_back(time);
        // recursive call
        int child_score = compute_minimax_search(next_automata_indexes, next_marked, depth-1, alpha, beta, false, time+1);
        if(child_score>best_score) best_action = action;
        best_score = max(child_score, best_score);
        // undo action
        visited_node_time.erase(node_to_string(automata_indexes, marked));
        if(marked) marked_visits.pop_back();

        if (beta<=best_score) break;
        alpha = max(alpha, best_score);
      }
      //cerr<<"actions controlled: "; for(auto&it:actions)cerr<<it<<"#"; cerr<<endl;
      //cerr<<"best score: "<<best_score<<", action: "<<best_action<<" ,for state "<<node_to_string(automata_indexes, marked)<<endl;
      return best_score;
    }
  }

  static bool is_marked_action(const string &action, vector<string> &_marked_actions) {
    for(auto&marked_action:_marked_actions) {
      // action marked_action prefix of action?
      if(marked_action.size()>action.size())continue;
      bool is_marked = true;
      for(int i=0;i<marked_action.size();i++) {
        if(action[i]!=marked_action[i]) {is_marked=false; break;}
      }
      if(is_marked)return true;
    }
    return false;
  }

private:

    vector<int> marked_visits = {-1};

  string node_to_string(vector<int> &automata_indexes, bool marked_state) {
    string ans = "";
    for(auto&it:automata_indexes) ans += to_string(it), ans += '-';
    if(marked_state) ans += 'M';
    return ans;
  }

  GameState getGameState(vector<int>& automata_indexes, bool marked) {
    string node = node_to_string(automata_indexes, marked);
    bool already_visited = visited_node_time.count(node);
    if (not already_visited) return GameState::IN_PROGRESS;
    // node already visited:
    if (marked) return GameState::MAXIMIZING_WON; // two passes on the same marked node
    // node already visited and not marked:
    int node_time = visited_node_time[node];
    if ( marked_visits.back() > node_time ) { // circuit containing one marked node
      return GameState::MAXIMIZING_WON;
    } else return GameState::MINIMIZING_WON; // circuit of all non-marked nodes
  }

  set<string> get_player_actions(vector<int>& automata_state_indexes, bool isMaximizingPlayer) {
    set<string> actions;
    for(int i=0;i<automata_state_indexes.size();i++) {
      vector<pair<vector<string>, vector<string>>> &state = automatas[i][automata_state_indexes[i]];
      for (auto &to_actions: state) {
        if (isMaximizingPlayer /* uncontrollable player */ ) {
          actions.insert(to_actions.first.begin(), to_actions.first.end());
        } else { /* controllable player */
          actions.insert(to_actions.second.begin(), to_actions.second.end());
        }
      }
    }
    return actions;
  }

  vector<int> get_automata_indexes_from_action(vector<int> &automata_state_indexes, const string &action, bool isMaximizingPlayer) {
    if (action==PASS_ACTION) return automata_state_indexes;
    vector<int> next_automata_indexes = automata_state_indexes;
    for(int i=0;i<automata_state_indexes.size();i++) {
      vector< pair<vector<string>, vector<string>> > & state = automatas[i][ automata_state_indexes[i] ];
      bool action_found = false;
      for(int to=0;to<state.size();to++) {
        if(action_found)break;
        if(isMaximizingPlayer) {
          for(auto&to_action:state[to].first) {
            if(to_action==action) {action_found=true; next_automata_indexes[i] = to; break;}
          }
        }else{
          for(auto&to_action:state[to].second) {
            if(to_action==action) {action_found=true; next_automata_indexes[i] = to; break;}
          }
        }
      }
    }
    return next_automata_indexes;
  }

};

int main() {
  string format;
  cin>>format; assert(format=="#Controllable");
  int _controlled_actions; cin>>_controlled_actions;
  vector<string> controlled_actions(_controlled_actions);
  for(auto&controlled_action:controlled_actions) cin>>controlled_action;
  cin>>format; assert(format=="#Marking");
  int _marked_actions; cin>>_marked_actions;
  vector<string> marked_actions(_marked_actions);
  for(auto&marked_action:marked_actions) cin>>marked_action;
  cin>>format; assert(format=="#Machines");
  int machines; cin>>machines;
  vector< vector<vector< pair<vector<string>, vector<string>> >> > automatas(machines);
  for(auto&automata:automatas) {
    string machine_name; cin>>machine_name; // ignore this for now
    cin>>format; assert(format=="#States");
    int states; cin>>states;
    automata = vector<vector< pair<vector<string>, vector<string>> >>(states+1, vector< pair<vector<string>, vector<string> >>(states+1));
    cin>>format; assert(format=="#Edges");
    int edges;cin>>edges;
    while(edges--){
      int from, to; string action; cin>>from>>to>>action;
      if(from==-1) from=states; if(to==-1) to=states;
      cerr<<"from: "<<from<<", to: "<<to<<" doing "<<action<<" marked: "<<miniMaxTreeSearch::is_marked_action(action, marked_actions)<<endl;
      if( miniMaxTreeSearch::is_marked_action(action, controlled_actions) ) // action is controlled or uncontrolled
        automata[from][to].first.push_back(action);
      else
        automata[from][to].second.push_back(action);
    }
  }
  miniMaxTreeSearch _miniMaxTreeSearch(automatas, marked_actions);
  _miniMaxTreeSearch.solve();

}

/*

  real    2m20,018s
  user    2m6,938s
  sys     0m13,019s

  (Ofast)
  real    1m45,177s
  user    1m31,348s
  sys     0m12,996s


 */