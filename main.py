import jpype.imports

from util import *
if not jpype.isJVMStarted():
    jpype.startJVM(classpath=['mtsa.jar'])
from MTSTools.ac.ic.doc.mtstools.model.operations.DCS.nonblocking import FeatureBasedExplorationHeuristic

# This files converts .fsp files from https://github.com/tdelgado00/Learning-Synthesis/tree/master/fsp
# into .txt files with all the automatas description for reconstruction

benchmark_metadata = {
    'AT': {
        'controllable': ['descend', 'approach'],
        'marking': ['control.all']
    },
    'BW': {
        'controllable': ['assign', 'refuse', 'approve'],
        'marking': ['refuse', 'approve']
    },
    'CM': {
        'controllable': ['mouse', 'move'],
        'marking': ['safe']
    },
    'DP': {
        'controllable': ['take'],
        'marking': ['eat.all']
    },
    'TA': {
        'controllable': ['cancel' , 'purchase', 'agency.succ', 'agency.fail'],
        'marking': ['agency.succ', 'agency.fail']
    },
    'TL': {
        'controllable': ['get'],
        'marking': ['accept', 'reject']
    }
}

if __name__ == "__main__":

    for n in range(1, 15+1):
        for k in range(1, 15+1):
            for benchmark in benchmark_metadata.keys():

                path = f'/home/asansone/Desktop/Learning-Synthesis/fsp/{benchmark}/{benchmark}-{n}-{k}.fsp'
                write_file = f'/home/asansone/Desktop/Learning-Synthesis/fsp_automatas/{benchmark}/{benchmark}-{n}-{k}.txt'
                f = open(write_file, "w")

                detached_initial_componentwise_info = FeatureBasedExplorationHeuristic.compileFSP(path).getFirst()
                f.write('#Machines\n')
                f.write(f'{len(detached_initial_componentwise_info.getMachines())}\n')
                for machine in detached_initial_componentwise_info.getMachines():
                    f.write(f"{str(machine).split(' -')[0]}\n")
                    f.write('#States\n')
                    f.write(f'{len(machine.states)}\n')
                    alphabet = machine.alphabet
                    f.write('#Edges\n')
                    edges = []
                    i = 0
                    for state in machine.states:
                        for elem in state.elements():
                            edges.append(f'{i} {elem.getNext()} {alphabet[elem.getEvent()]}')
                        i+=1
                    f.write(f'{len(edges)}\n')
                    for edge in edges:
                        f.write(f'{edge}\n')

                f.write('#Controllable\n')
                f.write(f"{len(benchmark_metadata[benchmark]['controllable'])}\n")
                for controllable in benchmark_metadata[benchmark]['controllable']:
                    f.write(f'{controllable}\n')
                f.write('#Marking\n')
                f.write(f"{len(benchmark_metadata[benchmark]['marking'])}\n")
                for marked in benchmark_metadata[benchmark]['marking']:
                    f.write(f'{marked}\n')   

                f.close()
