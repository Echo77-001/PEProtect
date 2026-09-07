#pragma once

#include<sstream>

#include "../vm/VMCodeBlock.h"
#include "../core/PEParser.h"

void DisassembleCodeBlock(PEParser& targetFile,const CodeBlock& block);