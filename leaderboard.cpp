#include "leaderboard.hpp"

// An action for creating new leader board
// owner - owner's wax account
// boardname - name for leaderboard
// gameid - id of owner's game
void leaderboard::createlboard(eosio::name owner, string boardname, uint64_t gameid)
{
    require_auth(owner);

    lboards _lb(CONTRACTN, CONTRACTN.value);

    uint64_t boardid{0};

    auto itr = _lb.end();
    if (itr == _lb.begin())
    {
        boardid = 0;
    }
    else
    {
        boardid = itr->id;
        boardid++;
    }

    _lb.emplace(CONTRACTN, [&](auto &new_board) {
        new_board.id = boardid;
        new_board.owner = owner;
        new_board.boardname = boardname;
        new_board.gameid = gameid;
    });
}

// An action for creating prize list for a leaderboard
// owner - owner of a wax account
// boardid - leaderboard id on table lboards
// mode - prize paments mode: 0 - absolute numbers, 1 - percents 
// values - string with payments values for top users on leaderboard
// values example - 100,50,40,
void leaderboard::createprize(eosio::name owner, uint64_t boardid, uint8_t mode, string values)
{
    require_auth(owner);

    lboards _lb(CONTRACTN, CONTRACTN.value);

    auto lb_itr = _lb.find(boardid);
    eosio::check(lb_itr != _lb.end(), "Leader board with " + to_string(boardid) + " id doesn't exist!");
    eosio::check(lb_itr->owner == owner, "You are not owner of current leaderboard.");

    vector<double> v; 
 
    while (!values.empty()) { 
        string substr; 
        size_t pos = values.find(',');
        substr = values.substr(0,pos);
        v.push_back(stoi(substr)); 
        values.erase(0,pos+1);
    } 
  

    _lb.modify(lb_itr, CONTRACTN, [&](auto &mod_lb) {
        for (int i = 0; i < v.size(); i++)
        {
            mod_lb.prize.values.push_back(v.at(i));
        }
        mod_lb.prize.mode = mode;
    });
}

// An action for removing leaderboard from table
// owner - owner wax account
// boardid - leaderboard id on table lboards
void leaderboard::removelboard(uint64_t boardid, eosio::name owner)
{
    require_auth(owner);

    lboards _lb(CONTRACTN, CONTRACTN.value);

    auto lb_itr = _lb.find(boardid);
    eosio::check(lb_itr != _lb.end(), "Leader board with " + to_string(boardid) + " id doesn't exist!");
    eosio::check(lb_itr->owner == owner, "You are not owner of current leaderboard.");

    _lb.erase(lb_itr);
}

// An action for reseting leaderboard
// owner - owner wax account
// boardid - leaderboard id on table lboards
// resetpool - 0: pot on leaderboard stay current, 1: pot on leaderboard change to 0
void leaderboard::resetlboard(eosio::name owner, uint64_t boardid, bool resetpool)
{
    require_auth(owner);

    lboards _lb(CONTRACTN, CONTRACTN.value);

    auto lb_itr = _lb.find(boardid);
    eosio::check(lb_itr != _lb.end(), "Leader board with " + to_string(boardid) + " id doesn't exist!");
    eosio::check(lb_itr->owner == owner, "You are not owner of current leaderboard.");

    _lb.modify(lb_itr, get_self(), [&](auto &mod_board) {
        mod_board.players.clear();
        if (resetpool == 1) 
        mod_board.pot = 0.0;
    });
}

// An action for update/create a user on leaderboard
// owner - owner wax account
// boardid - leaderboard id on table lboards
// username - user's wax account
// point - number of points to add
// data - user data on JSON format
void leaderboard::update(eosio::name owner, uint64_t boardid, eosio::name username, double points, string data)
{
    require_auth(owner);

    lboards _lb(CONTRACTN, CONTRACTN.value);

    auto lb_itr = _lb.find(boardid);
    eosio::check(lb_itr != _lb.end(), "Leader board with " + to_string(boardid) + " id doesn't exist!");
    eosio::check(lb_itr->owner == owner, "You are not owner of current leaderboard.");

    uint64_t pos = finder(lb_itr->players, username);
    if (pos == -1)
    {
        player_s helper;
        pos = lb_itr->players.size();
        _lb.modify(lb_itr, get_self(), [&](auto &mod_board) {
            mod_board.players.push_back({
                username,
                points,
                data,
            });
            while (pos != 0)
            {
                if (mod_board.players.at(pos).points > mod_board.players.at(pos - 1).points)
                {
                    helper = mod_board.players.at(pos);
                    mod_board.players.at(pos) = mod_board.players.at(pos - 1);
                    mod_board.players.at(pos - 1) = helper;
                }
                else
                {
                    break;
                }
            }
        });
    }
    else
    {
        player_s helper;
        _lb.modify(lb_itr, get_self(), [&](auto &mod_board) {
            mod_board.players.at(pos).points += points;
            while (pos != 0)
            {
                if (mod_board.players.at(pos).points > mod_board.players.at(pos - 1).points)
                {
                    helper = mod_board.players.at(pos);
                    mod_board.players.at(pos) = mod_board.players.at(pos - 1);
                    mod_board.players.at(pos - 1) = helper;
                }
                else
                {
                    break;
                }
            }
        });
    }
}

uint64_t leaderboard::finder(vector<player_s> players, eosio::name username)
{
    for (uint64_t i = 0; i < players.size(); i++)
    {
        if (players.at(i).account == username)
        {
            return i;
        }
    }
    return -1;
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
  if (code == receiver)
  {
    switch (action)
    {
      EOSIO_DISPATCH_HELPER(leaderboard, (createlboard)(createprize)(resetlboard)(removelboard)(update))
    }
  }
  else
  {
    printf("Couldn't find an action.");
    return;
  }
}
