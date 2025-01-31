#include "lsystem.h"

float sdRoundCone(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r1, float r2);
float sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r);
void MoveForward();
float sdTriPrism(glm::vec3 p, glm::vec2 h);

LSystem::LSystem(Turtle& currturtle)
{
    currTurtle = mkU<Turtle>(currturtle);
    AddDefaultFuncPointer();
}

void LSystem::AddRule(char a_refChar, Rule a_postCondition)
{
    RulesList.insert(std::make_pair(a_refChar, a_postCondition));
}

void LSystem::AddFuncPointer(char a_refChar, RuleFunc a_funcPointer)
{
    Func_Pointer.insert(std::make_pair(a_refChar, a_funcPointer));
}

void LSystem::LSystemParse(int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        Symbol* currSym = rootSymbol;
        while (currSym != nullptr)
        {
            if (RulesList.find(currSym->m_refCharacter) != RulesList.end())
            {
                Symbol* nextSym = currSym->next;
                ApplyRule(currSym->prev, currSym, nextSym, currSym->m_refCharacter, i + 1);

                currSym = nextSym;
                continue;
            }
            currSym = currSym->next;
        }
    }
}

void LSystem::PrintParsedSystem()
{
    Symbol* currSym = rootSymbol;
    if (currSym != NULL)
    {
        while (currSym != NULL)
        {
            std::cout << currSym->m_refCharacter;
            currSym = currSym->next;
        }
    }
}

void LSystem::ClearParsedString()
{
    SymbolList.clear();
    rootSymbol = nullptr;
}

void LSystem::AssignAxiom(std::string a_refAxiom)
{
    axiom = a_refAxiom;
    sPtr<Symbol> m_rootSymbol = mkS<Symbol>(a_refAxiom[0], 0);
    rootSymbol = m_rootSymbol.get();
    SymbolList.push_back(std::move(m_rootSymbol));
    if (a_refAxiom.length() <= 1)
    {
        return;
    }
    Symbol* prevSym = rootSymbol;

    for (int i = 1; i < a_refAxiom.length(); i++)
    {
        char c = a_refAxiom[i];
        sPtr<Symbol> newSymbol = mkS<Symbol>(c, 0);
        prevSym->next = newSymbol.get();
        newSymbol->prev = prevSym;
        prevSym = prevSym->next;
        SymbolList.push_back(std::move(newSymbol));
    }
}

void LSystem::ApplyRule(Symbol* a_prevSym, Symbol* a_currSym, Symbol* a_endSym, char a_ruleKey, int iteration)
{
    Symbol* prevSym = a_prevSym;
    Symbol* del_Symbol = a_currSym;


    //delete del_Symbol;
    float randNumber = (std::rand() % 100) / 100.0f;
    Rule currRule(RulesList[a_ruleKey]);
    PostCondition randomRule = currRule.GetRandomRule(randNumber);
    std::string RuleValue = randomRule.new_symbol;

    if (a_currSym == rootSymbol)
    {
        sPtr<Symbol> newSymbol = mkS<Symbol>(RuleValue[0], iteration);
        rootSymbol = newSymbol.get();
        prevSym = newSymbol.get();
        SymbolList.push_back(std::move(newSymbol));

        for (int i = 1; i < RuleValue.length(); i++)
        {
            sPtr<Symbol> newSymbol = mkS<Symbol>(RuleValue[i], iteration);
            prevSym->next = newSymbol.get();
            newSymbol->prev = prevSym;
            prevSym = newSymbol.get();
            SymbolList.push_back(std::move(newSymbol));
        }
    }
    else
    {
        for (int i = 0; i < RuleValue.length(); i++)
        {
            sPtr<Symbol> newSymbol = mkS<Symbol>(RuleValue[i], iteration);
            prevSym->next = newSymbol.get();
            newSymbol->prev = prevSym;
            prevSym = newSymbol.get();
            SymbolList.push_back(std::move(newSymbol));
        }
    }
    prevSym->next = a_endSym;
    if (a_endSym != nullptr)
    {
        a_endSym->prev = prevSym;
    }
}

void LSystem::AddDefaultFuncPointer()
{
    AddFuncPointer('F', &Turtle::Move);
    AddFuncPointer('[', &Turtle::SaveState);
    AddFuncPointer(']', &Turtle::LoadState);
    AddFuncPointer('+', &Turtle::TurnRight);
    AddFuncPointer('-', &Turtle::TurnLeft);
}

void MoveForward(Turtle(*currTurtle))
{
    std::cout << "Move Forward" << std::endl;
    currTurtle->Move();
}

bool ContainsPos(std::vector<glm::vec3> &posBuffer, glm::vec3 a_position)
{
    for (int i = 0; i < posBuffer.size(); i++)
    {
        if (posBuffer[i] == a_position)
        {
            return true;
        }
    }
    return false;
}


inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

int CreateNewMaterial(std::vector<Material> &materials) {
        

        int start = materials.size();
        //load static properties
        for (int i = start; i < start +  3; i++) {
            Material newMaterial;
            newMaterial.color = glm::vec3(random_double(), random_double(), random_double());
            newMaterial.isSubSurface = false;
            
            newMaterial.specular.exponent = random_double();
            newMaterial.specular.color = glm::vec3(random_double(), random_double(), random_double());
            double reflect = random_double();
            if (reflect > 0.7)
            {
                newMaterial.hasReflective = 1;
            }
            else
            {
                newMaterial.hasReflective = 0;
            }
            if (i == start)
            {
                //newMaterial.hasRefractive = 1;
                newMaterial.usingProcTex = 1;
                newMaterial.ProcTexNum = 1;
            }
           
            newMaterial.indexOfRefraction = 1 + reflect;
            
                newMaterial.emittance = 0;
                materials.push_back(newMaterial);
        }
        return start;
    }


Geom createNewGeom(glm::vec3 currPos, int materialID, enum GeomType type) {
        Geom newGeom;
        newGeom.type = type;
        newGeom.materialid = materialID;

        newGeom.translation = currPos;
        newGeom.rotation = glm::vec3(0, 0, 0);
        newGeom.scale = glm::vec3(0.5, 0.5, 0.5);
        newGeom.transform = utilityCore::buildTransformationMatrix(
            newGeom.translation, newGeom.rotation, newGeom.scale);
        newGeom.inverseTransform = glm::inverse(newGeom.transform);
        newGeom.invTranspose = glm::inverseTranspose(newGeom.transform);
        return newGeom;
}


void LSystem::CarveBuilding(std::vector<glm::vec3> &procShape, std::vector<Geom> &geoms, std::vector<Material>& materials, enum GeomType type)
{
    Symbol* currSym = rootSymbol;
    glm::vec3 a = currTurtle->m_Position;
    glm::vec3 b = currTurtle->m_Position;

    int startIdxMat = CreateNewMaterial(materials);
    float r1 = currTurtle->radius;
    float r2 = currTurtle->radius;

    while (currSym != nullptr)
    {
        if (Func_Pointer.find(currSym->m_refCharacter) != Func_Pointer.end())
        {
            RuleFunc function = Func_Pointer[currSym->m_refCharacter];
            if (currSym->m_refCharacter == 'F')
            {
                r1 = currTurtle->radius;
                a = currTurtle->m_Position;
            }
            (currTurtle.get()->*function)();
            if (currSym->m_refCharacter == 'F')
            {
                //currTurtle->radius = currTurtle->radius - 0.1f;
                b = currTurtle->m_Position;
                r2 = currTurtle->radius;
                for (int x = -r1; x <= +r1; x++)
                {
                    for (int y = -r1; y <= r1; y++)
                    {
                        for (int z = 0; z <= currTurtle->lambda; z++)
                        {
                            //glm::mat3 rotMat = glm::mat3(currTurtle->m_Right, currTurtle->m_Up, currTurtle->m_forward);
                            glm::vec3 currpos = a + float(z) * currTurtle->m_forward;
                            currpos = currpos + glm::vec3(x, 0.0f, y);
                            //glm::vec3 currPos = a + glm::vec3(x, y, z) * rotMat;
                            if ((sdCapsule(currpos, a, b, r1) <= 0))
                            {
                                if (!ContainsPos(procShape, currpos))
                                {
                                    auto choose_mat = random_double();
                                    procShape.push_back(currpos);
                                    if (choose_mat < 0.8) {
                                        geoms.push_back(createNewGeom(currpos, startIdxMat, type));
                                    }
                                    else if (choose_mat < 0.95) {
                                        geoms.push_back(createNewGeom(currpos, startIdxMat + 1, type));
                                    }
                                    else {
                                        geoms.push_back(createNewGeom(currpos, startIdxMat + 2, type));
                                    }
                                }
                            }

                        }
                    }

                }
            }
        }

        //std::cout<<currSym->m_refCharacter;
        currSym = currSym->next;

    }
}

float dot2(glm::vec2 v) { return glm::dot(v, v); }
float dot2(glm::vec3 v) { return glm::dot(v, v); }

float sdRoundCone(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r1, float r2)
{
    // sampling independent computations (only depend on shape)
    glm::vec3  ba = b - a;
    float l2 = glm::dot(ba, ba);
    float rr = r1 - r2;
    float a2 = l2 - rr * rr;
    float il2 = 1.0 / l2;

    // sampling dependant computations
    glm::vec3 pa = p - a;
    float y = glm::dot(pa, ba);
    float z = y - l2;
    float x2 = dot2(pa * l2 - ba * y);
    float y2 = y * y * l2;
    float z2 = z * z * l2;

    // single square root!
    float k = glm::sign(rr) * rr * rr * x2;
    if (glm::sign(z) * a2 * z2 > k) return  sqrt(x2 + z2) * il2 - r2;
    if (glm::sign(y) * a2 * y2 < k) return  sqrt(x2 + y2) * il2 - r1;
    return (sqrt(x2 * a2 * il2) + y * rr) * il2 - r1;
}


float sdTriPrism(glm::vec3 p, glm::vec2 h)
{
    glm::vec3 q = glm::abs(p);
    return glm::max(q.z - h.y, glm::max(q.x * 0.866025f + p.y * 0.5f, -p.y) - h.x * 0.5f);
}

float sdCapsule(glm::vec3 p, glm::vec3 a, glm::vec3 b, float r)
{
    glm::vec3 pa = p - a, ba = b - a;
    float h = glm::clamp(glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f);
    return glm::length(pa - ba * h) - r;
}
