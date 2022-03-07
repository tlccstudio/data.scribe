
//**************************** PUBLIC ACTIONS

ACTION datascribe::message(name sender, string msg) {
  require_auth(sender);
  
  msg_proc(sender, msg);
};

//**************************** PRIVATE FUNCTIONS

void datascribe::msg_proc(name sender, string msg) {

  check(msg.size() <= 5000, "msg parameter should be no more than 5000 characters.");

  vector<string> aMsg = split(msg,"|!|");
  string sData = "";
  uint16_t count = 0;

  check((aMsg.size() % 2) == 1, "Invalid format using wrong quantity of |!| within message.");
  check(aMsg.size() <= 20, "This message contains more than 10 variable updates.");
  //formats:
  //|!|var_name|vtype|value|!| ---  will perform set operation at index 0
  //|!|var_name|vtype|value|operation|index|!| --- full variable setting
  //|!|var_name|vtype|value|operation|index|vardgt(regvar)|type(regvar)|tlimit|vlimit|!|  --- includes registration details
  
  vector<string> aVar;
  name varname;
  string oper;
  string vtype;
  uint8_t index;
  string value;

  //optional vars
  name vardgt;
  string type;
  uint64_t tlimit;
  uint8_t vlimit;
  
  //varname vars
  varreg_index _varreg(get_self(), sender.value);
  auto itr_varreg = _varreg.find(NAME_NULL);
 
  for(uint16_t i=1; i < aMsg.size(); (i+=2)) {
    vector<string>    sval;
    vector<uint128_t> uval;
    vector<int128_t>  nval;

    aVar = split(aMsg[i],"|");
    check((aVar.size() >= 3) || (aVar.size() == 5) || (aVar.size() == 9), "Variable format error when using | character.");

    varname = name(aVar[0]);
    vtype   = aVar[1];
    value = aVar[2];

    if(aVar.size() == 9) {
      oper    = aVar[3];
      index   = stoi(aVar[4]);
      vardgt  = name(aVar[5]);
      type    = aVar[6];
      tlimit  = (uint64_t) stoi(aVar[7]);
      vlimit  = (uint8_t) stoi(aVar[8]);
    } else if(aVar.size() == 5) {
      oper   = aVar[3];
      index   = stoi(aVar[4]);
    } else {
      oper = "set";
      index = 0;
    }

    if(vtype == "u") { uval.push_back(stoi(value)); }
    else if(vtype == "n") { nval.push_back(stoi(value)); }
    else if(vtype == "s") { sval.push_back(value); }
    
    //find varnames
    itr_varreg = _varreg.find(varname.value);
    if((itr_varreg == _varreg.end()) && (aVar.size() == 9)) { //if varname is not registered, and registration details provided
      _regvar(sender, sender, varname, vardgt, type, tlimit, vlimit);
    }

    _update(sender, sender, varname, oper, index, uval, sval, nval);

    /*  // Old version when was separate contract
    action(
        permission_level{ get_self(), "active"_n},
        name("var.scribe"),
        "update"_n,
        std::make_tuple(
          get_self(), //signor
          sender, //scope
          varname, //varname
          oper,  //operation
          index, //index
          uval, //uval
          sval, //sval
          nval //nval
        )
    ).send();*/
  }

};
