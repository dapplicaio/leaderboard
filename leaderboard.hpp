#pragma once
#include <eosio/eosio.hpp>
#include <eosio/print.hpp>
#include <eosio/asset.hpp>
#include <eosio/symbol.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include <eosio/singleton.hpp>

using namespace std;

#define CONTRACTN eosio::name("leaderboard1")

CONTRACT leaderboard : public eosio::contract {
  public:
    using contract::contract;

    ACTION createlboard(eosio::name owner, string boardname, uint64_t gameid);
    ACTION createprize(eosio::name owner, uint64_t boardid, uint8_t mode, string value);
    ACTION resetlboard(eosio::name owner, uint64_t boardid, bool resetpool);
    ACTION removelboard(uint64_t boardid, eosio::name owner);
    ACTION update(eosio::name owner, uint64_t boardid, eosio::name username, double points, string data);

  private:

    struct player_s
    {
      eosio::name account;      //user wax account
      double points;            //user points
      string data;              //user data on JSON
      double payout;            //user payouts
    };

    struct prize_s
    {
      uint8_t mode;             //prize mode: 0 - absolute numbers, 1 - percents
      vector<double> values;    //users' prizes parts
    };

    TABLE lboard {
      uint64_t          id;         //leaderboard id on table
      eosio::name       owner;      //owner wax account name
      string            boardname;  //name of current board
      uint64_t          gameid;     //game's id
      double            pot;        //total pot on current leaderboard
      vector<player_s>  players;    //array of players
      prize_s           prize;      //array and type of prizes

      uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index<eosio::name("lboards"), lboard> lboards;

  public:
  
  leaderboard(eosio::name receiver, eosio::name code, eosio::datastream<const char *> ds) : contract(receiver, code, ds) {}
  uint64_t finder(vector<player_s> players, eosio::name username);
   
};
