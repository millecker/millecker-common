/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hama/Pipes.hh"
#include "hama/TemplateFactory.hh"
#include "hadoop/StringUtils.hh"

#include<stdlib.h>
#include<string>
#include<iostream>

using std::string;
using std::cout;
using std::cerr;

using HamaPipes::BSP;
using HamaPipes::BSPContext;
using namespace HadoopUtils;

class SumBSP: public BSP<string,string,string,double,double> {
private:
  string masterTask;
public:
  SumBSP(BSPContext<string,string,string,double,double>& context) {  }
  
  void setup(BSPContext<string,string,string,double,double>& context) {
    // Choose one as a master
    masterTask = context.getPeerName(context.getNumPeers() / 2);
  }
  
  void bsp(BSPContext<string,string,string,double,double>& context) {
    
    double intermediateSum = 0.0;
    // key and value are strings because of KeyValueTextInputFormat
    string key;
    string value;
    
    while(context.readNext(key,value)) {
      cout << "SumBSP bsp: key: " << key << " value: "  << value  << "\n";
      intermediateSum += toDouble(value);
    }
    
    cout << "SendMessage to Master: " << masterTask << " value: "  << intermediateSum  << "\n";
    context.sendMessage(masterTask, intermediateSum);
    context.sync();
  }
  
  void cleanup(BSPContext<string,string,string,double,double>& context) {
    if (context.getPeerName().compare(masterTask)==0) {
      cout << "I'm the MasterTask fetch results!\n";
      double sum = 0.0;
      int msgCount = context.getNumCurrentMessages();
      cout << "MasterTask fetches " << msgCount << " results!\n";
      for (int i=0; i<msgCount; i++) {
        sum += context.getCurrentMessage();
      }
      cout << "Sum " << sum << " write results...\n";
      context.write("Sum", sum);
    }
  }
};

int main(int argc, char *argv[]) {
  return HamaPipes::runTask<string,string,string,double,double>(HamaPipes::TemplateFactory<SumBSP,string,string,string,double,double>());
}
