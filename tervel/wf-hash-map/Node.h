#ifndef TERVEL_WFHM_NODE_H
#define TERVEL_WFHM_NODE_H

class Node {
public:
  virtual bool IsPairNode() = 0;
  virtual bool IsArrayNode() = 0;
};

