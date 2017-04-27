/*BEGIN_LEGAL
Intel Open Source License

Copyright (c) 2002-2016 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <set>
#include "pin.H"
#define BUF_SIZE 256

FILE * trace;
PIN_LOCK lock;

bool instrument = false;

/* ===================================================================== */
// Command line switches
/* ===================================================================== */
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE,  "pintool",
    "o", "trace_file.out", "specify file name for MyPinTool output");

//KNOB<string> KnobFunctionsFile(KNOB_MODE_WRITEONCE,  "pintool",
//    "f", "funcs.in", "specify file name for reading functions");

//std::set<std::string> funcs;

// Print a memory read record
VOID RecordMemRead(VOID * ip, VOID * addr, THREADID tid)
{
    if(instrument) {
        PIN_GetLock(&lock, tid+1);
        fprintf(trace,"%d R %p\n", tid, addr);
        fflush(trace);
        PIN_ReleaseLock(&lock);
    }
}

// Print a memory write record
VOID RecordMemWrite(VOID * ip, VOID * addr, THREADID tid)
{
    if(instrument) {
        PIN_GetLock(&lock, tid+1);
        fprintf(trace,"%d W %p\n", tid, addr);
        fflush(trace);
        PIN_ReleaseLock(&lock);
    }
}

// Is called for every instruction in the routine
VOID InstrumentInstruction(INS ins)
{
    // Instruments memory accesses using a predicated call, i.e.
    // the instrumentation is called iff the instruction will actually be executed.
    //
    // On the IA-32 and Intel(R) 64 architectures conditional moves and REP
    // prefixed instructions appear as predicated instructions in Pin.
    UINT32 memOperands = INS_MemoryOperandCount(ins);

    // Iterate over each memory operand of the instruction.
    for (UINT32 memOp = 0; memOp < memOperands; memOp++)
    {
        if (INS_MemoryOperandIsRead(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_THREAD_ID,
                IARG_END);
        }
        // Note that in some architectures a single memory operand can be
        // both read and written (for instance incl (%eax) on IA-32)
        // In that case we instrument it once for read and once for write.
        if (INS_MemoryOperandIsWritten(ins, memOp))
        {
            INS_InsertPredicatedCall(
                ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
                IARG_INST_PTR,
                IARG_MEMORYOP_EA, memOp, IARG_THREAD_ID,
                IARG_END);
        }
    }
}

/**
 * When this routine comes in, we start analyzing instructions for memory traces
 */
VOID StartInstr() {
    cout << "start\n";
    instrument = true;
}

/**
 * When this routine comes in, we stop analyzing instructions for memory traces
 */
VOID StopInstr() {
    cout << "stop\n";
    instrument = false;
}

VOID Routine(RTN rtn, VOID *v) {
    std::string name = PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY);
    RTN_Open(rtn);

    /* Count instructions only for demarcated part of program */
    if(name == "dummy_instr_start") {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)StartInstr, IARG_END);
    } else if(name == "dummy_instr_end") {
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)StopInstr, IARG_END);
    } else {

        /* Loop over all instructions in routine */
        for(INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins)) {
            InstrumentInstruction(ins);
        }
    }

    RTN_Close(rtn);
}

VOID Fini(INT32 code, VOID *v)
{
    fclose(trace);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR( "This Pintool prints a trace of memory addresses\n"
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/**
 * Fill funcs set
 * @param file_name - The name of the file to open for reading functions file
 */
/*void fill_funcs(std::string file_name) {
    ifstream file(file_name.c_str());
    std::string func_name;

    while(file >> func_name) {
        funcs.insert(func_name);
    }
}*/


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
    PIN_InitSymbols();

    string fileName = KnobOutputFile.Value();

    //fill_funcs(KnobFunctionsFile.Value());
    trace = fopen(fileName.c_str(), "w");
    if(trace == NULL) {
        perror("Open");
        return -1;
    }

    PIN_InitLock(&lock);

    RTN_AddInstrumentFunction(Routine, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
}
