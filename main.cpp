#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace wb {
struct Vec3 { double x{},y{},z{}; };
struct Triangle { Vec3 a,b,c; };
struct Node { std::int64_t id; Vec3 p; double t; };

std::vector<Triangle> read_ascii_stl(const std::string& path) {
    if(std::filesystem::file_size(path)>20u*1024u*1024u) throw std::runtime_error("ASCII STL exceeds tested 20 MiB limit");
    std::ifstream in(path); if(!in) throw std::runtime_error("Cannot open STL: "+path);
    std::vector<Triangle> out; std::string line; std::array<Vec3,3> v{}; int n=0;
    while(std::getline(in,line)) { std::istringstream ss(line); std::string key; ss>>key;
        if(key=="vertex") { if(!(ss>>v[n].x>>v[n].y>>v[n].z) || !std::isfinite(v[n].x)||!std::isfinite(v[n].y)||!std::isfinite(v[n].z)) throw std::runtime_error("Malformed/non-finite STL vertex"); if(++n==3){out.push_back({v[0],v[1],v[2]}); n=0;} }
    }
    if(out.empty()) throw std::runtime_error("STL contains no triangles");
    if(n) throw std::runtime_error("STL ends with an incomplete triangle");
    return out;
}
std::vector<Node> read_temperature_csv(const std::string& path) {
    if(std::filesystem::file_size(path)>20u*1024u*1024u) throw std::runtime_error("Temperature CSV exceeds tested 20 MiB limit");
    std::ifstream in(path); if(!in) throw std::runtime_error("Cannot open CSV: "+path);
    std::string line; if(!std::getline(in,line) || line!="node_id,x_m,y_m,z_m,temperature_C") throw std::runtime_error("Expected header node_id,x_m,y_m,z_m,temperature_C");
    std::vector<Node> out; while(std::getline(in,line)){ if(line.empty()) continue; std::replace(line.begin(),line.end(),',',' '); std::istringstream s(line); Node p{}; if(!(s>>p.id>>p.p.x>>p.p.y>>p.p.z>>p.t)) throw std::runtime_error("Malformed CSV row"); std::string extra; if(s>>extra) throw std::runtime_error("Too many CSV columns"); if(!std::isfinite(p.p.x)||!std::isfinite(p.p.y)||!std::isfinite(p.p.z)||!std::isfinite(p.t)) throw std::runtime_error("Non-finite CSV value"); out.push_back(p); }
    if(out.empty()) throw std::runtime_error("CSV contains no result nodes");
    return out;
}
struct Case { double k,q; std::size_t n; };
struct Solution { std::vector<double>x,t; };
Solution solve(const Case& c) {
    if(!(c.k>0) || c.n==0) throw std::invalid_argument("k and element count must be positive");
    constexpr double L=1.,A=1e-4,T0=100.,TL=20.; const std::size_t m=c.n+1; const double dx=L/c.n;
    std::vector<double>d(m),u(m),l(m),r(m);
    for(std::size_t e=0;e<c.n;++e){const auto i=e,j=e+1; const double s=c.k*A/dx, f=c.q*A*dx/2.; d[i]+=s;d[j]+=s;u[i]-=s;l[j]-=s;r[i]+=f;r[j]+=f;}
    // Eliminate prescribed end values in the assembled system.
    r[1]-=l[1]*T0; l[1]=0; r[m-2]-=u[m-2]*TL;u[m-2]=0;
    d[0]=1;u[0]=0;r[0]=T0; d[m-1]=1;l[m-1]=0;r[m-1]=TL;
    for(std::size_t i=1;i<m;++i){double f=l[i]/d[i-1];d[i]-=f*u[i-1];r[i]-=f*r[i-1];}
    std::vector<double>t(m);t[m-1]=r[m-1]/d[m-1];for(std::size_t i=m-1;i-->0;)t[i]=(r[i]-u[i]*t[i+1])/d[i];
    std::vector<double>x(m);for(std::size_t i=0;i<m;++i)x[i]=i*dx; return {x,t};
}
double exact(double x,double k=10.,double q=10000.) { return 100.-80.*x+(q/(2*k))*x*(1.-x); }
double midpoint_error(const Solution&s,double k=10.,double q=10000.) { double e=0;for(std::size_t i=0;i+1<s.x.size();++i){double x=(s.x[i]+s.x[i+1])/2, t=(s.t[i]+s.t[i+1])/2;e=std::max(e,std::abs(t-exact(x,k,q)));}return e; }
void write_solution(const std::string&p,const Solution&s){std::ofstream o(p);if(!o)throw std::runtime_error("Cannot write "+p);o<<"x_m,temperature_FE_C,temperature_exact_C\n"<<std::setprecision(12);for(std::size_t i=0;i<s.x.size();++i)o<<s.x[i]<<','<<s.t[i]<<','<<exact(s.x[i])<<'\n';}
struct Pool { std::mutex m; std::condition_variable cv; std::queue<std::function<void()>> q; bool stop=false; std::vector<std::thread> ts;
 explicit Pool(unsigned n){for(unsigned i=0;i<n;++i)ts.emplace_back([this]{for(;;){std::function<void()> f;{std::unique_lock<std::mutex> l(m);cv.wait(l,[&]{return stop||!q.empty();});if(stop&&q.empty())return;f=std::move(q.front());q.pop();}f();}});}
 template<class F> void submit(F f){{std::lock_guard<std::mutex>l(m);q.emplace(f);}cv.notify_one();}
 ~Pool(){{std::lock_guard<std::mutex>l(m);stop=true;}cv.notify_all();for(auto&t:ts)t.join();}
};
std::vector<Case> cases(std::size_t count,std::size_t n){std::mt19937 g(20261026);std::uniform_real_distribution<double>k(5.,25.),q(1000.,20000.);std::vector<Case>v;while(v.size()<count)v.push_back({k(g),q(g),n});return v;}
std::vector<Solution> batch(const std::vector<Case>&cs,unsigned workers){std::vector<Solution> r(cs.size());if(workers<=1){for(size_t i=0;i<cs.size();++i)r[i]=solve(cs[i]);}else{Pool p(workers);for(size_t i=0;i<cs.size();++i)p.submit([&,i]{r[i]=solve(cs[i]);});}return r;}
struct NN {double h,yhat,loss,dw,db,dv,dc;};
NN nn(double w=.5,double b=0,double v=1,double c=0){double h=std::tanh(w+b),yh=v*h+c,e=yh-2;double g=e*v*(1-h*h);return{h,yh,.5*e*e,g,g,e*h,e};}
void test(){for(size_t n:{10u,20u,40u,80u}){auto s=solve({10,10000,n});std::cout<<"elements="<<n<<" midpoint_max_error_C="<<std::setprecision(10)<<midpoint_error(s)<<"\n";}auto a=nn();const double eps=1e-6;std::array<double,4> an{a.dw,a.db,a.dv,a.dc},x{.5,0,1,0};for(int j=0;j<4;++j){auto p=x,m=x;p[j]+=eps;m[j]-=eps;double fd=(nn(p[0],p[1],p[2],p[3]).loss-nn(m[0],m[1],m[2],m[3]).loss)/(2*eps);if(std::abs(fd-an[j])>1e-5)throw std::runtime_error("NN finite-difference check failed");std::cout<<"gradient["<<j<<"]="<<an[j]<<" finite_difference="<<fd<<"\n";}}
void write_training(const std::string& csv,const std::string& svg){
    std::ofstream o(csv); if(!o) throw std::runtime_error("Cannot write "+csv);
    o<<"step,loss,w,b,v,c\n"<<std::setprecision(12);
    double w=.5,b=0,v=1,c=0; o<<0<<','<<nn(w,b,v,c).loss<<','<<w<<','<<b<<','<<v<<','<<c<<'\n';
    std::vector<double> losses{nn(w,b,v,c).loss};
    for(int i=1;i<=100;++i){auto a=nn(w,b,v,c);w-=.1*a.dw;b-=.1*a.db;v-=.1*a.dv;c-=.1*a.dc;auto z=nn(w,b,v,c);losses.push_back(z.loss);o<<i<<','<<z.loss<<','<<w<<','<<b<<','<<v<<','<<c<<'\n';}
    double mx=*std::max_element(losses.begin(),losses.end());
    std::ofstream g(svg); if(!g) throw std::runtime_error("Cannot write "+svg);
    g<<"<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"900\" height=\"500\" viewBox=\"0 0 900 500\"><rect width=\"100%\" height=\"100%\" fill=\"#101827\"/><text x=\"55\" y=\"35\" fill=\"white\" font-family=\"sans-serif\" font-size=\"22\">Single-sample backpropagation loss (100 updates)</text><path d=\"M70 60 V420 H850\" fill=\"none\" stroke=\"#93a4bb\"/><text x=\"380\" y=\"465\" fill=\"#d5deea\" font-family=\"sans-serif\">Training step</text><text x=\"15\" y=\"245\" fill=\"#d5deea\" font-family=\"sans-serif\" transform=\"rotate(-90 15 245)\">Loss J</text><polyline fill=\"none\" stroke=\"#50d2aa\" stroke-width=\"3\" points=\"";
    for(std::size_t i=0;i<losses.size();++i){double x=70+780.0*i/100,y=420-340*losses[i]/mx;g<<x<<','<<y<<' ';}
    g<<"\"/></svg>";
}
void demo(){std::cout<<"Unified Engineering Workbench - synthetic demo\n";test();for(size_t n:{10u,20u,40u,80u}){auto s=solve({10,10000,n});std::ostringstream name;name<<"results/fea_N"<<n<<".csv";write_solution(name.str(),s);}write_training("results/nn_loss.csv","results/nn_loss.svg");std::cout<<"Wrote four FEA CSV files and 100-step NN training data/plot.\n";}
}
int main(int argc,char**argv){try{if(argc<2){std::cout<<"Usage: WorkbenchCLI.exe --demo | --solve N output.csv | --import-stl file | --import-csv file | --bench cases mesh\n";return 0;}std::string cmd=argv[1];if(cmd=="--demo"){wb::demo();return 0;}if(cmd=="--solve"&&argc==4){auto n=std::stoul(argv[2]);auto s=wb::solve({10,10000,n});wb::write_solution(argv[3],s);std::cout<<"max midpoint error C="<<wb::midpoint_error(s)<<"\n";return 0;}if(cmd=="--import-stl"&&argc==3){auto v=wb::read_ascii_stl(argv[2]);std::cout<<"Triangles: "<<v.size()<<" (units follow source; no implicit scale)\n";return 0;}if(cmd=="--import-csv"&&argc==3){auto v=wb::read_temperature_csv(argv[2]);std::cout<<"Nodes: "<<v.size()<<" temperature_C range: ";auto mm=std::minmax_element(v.begin(),v.end(),[](auto&a,auto&b){return a.t<b.t;});std::cout<<mm.first->t<<" .. "<<mm.second->t<<"\n";return 0;}if(cmd=="--bench"&&argc==4){auto cs=wb::cases(std::stoul(argv[2]),std::stoul(argv[3]));std::vector<wb::Solution> serial;std::vector<std::pair<unsigned,double>> medians;unsigned cap=std::max(1u,std::thread::hardware_concurrency());for(unsigned p=1;p<=std::min(4u,cap);++p){std::vector<double>times;for(int rep=0;rep<6;++rep){auto t=std::chrono::steady_clock::now();auto r=wb::batch(cs,p);auto d=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-t).count();if(rep==0){if(p==1)serial=r;continue;}times.push_back(d);if(serial.size()!=r.size())throw std::runtime_error("batch size mismatch");for(size_t i=0;i<r.size();++i)for(size_t j=0;j<r[i].t.size();++j)if(std::abs(serial[i].t[j]-r[i].t[j])>1e-8)throw std::runtime_error("serial/parallel mismatch");}std::sort(times.begin(),times.end());medians.push_back({p,times[times.size()/2]});}double t1=medians.front().second;for(auto [p,tp]:medians)std::cout<<p<<" workers median_ms="<<tp<<" speedup="<<t1/tp<<" efficiency="<<(t1/tp/p)<<"\n";return 0;}std::cerr<<"Invalid command or arguments\n";return 2;}catch(const std::exception&e){std::cerr<<"Error: "<<e.what()<<"\n";return 1;}}
