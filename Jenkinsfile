pipeline {
    agent none
    environment {
        PIP_NO_INPUT = 1
    }
    options {
        timeout(time: 30, unit: 'MINUTES')
    }
    stages {
        stage ('Build and Test') {
            parallel {
                stage ('Linux Build and Test') {
                    agent {
                        dockerfile {
                            filename 'Dockerfile'
                            args '--cap-add SYS_PTRACE'
                        }
                    }
                    environment {
                        PYTHONUSERBASE = '/tmp/python/'
                        XDG_CACHE_HOME = '/tmp/.cache/pip'
                    }
                    steps {
                        checkout scm
                        sh './build.sh'
                        stash name: 'build-artifacts-linux', includes: 'dist/*.whl, dist/*.tar.gz'
                        archiveArtifacts artifacts: 'dist/*.whl, dist/*.tar.gz'
                    }
                }
                stage ('Windows Build py36') {
                    agent { label 'windows' }
                    steps {
                        deleteDir()
                        checkout scm
                        powershell './win.bat 36'
                        powershell './win.bat 36 64'
                        stash name: 'build-artifacts-win-py36', includes: 'dist/*.whl'
                        archiveArtifacts artifacts: 'dist/*.whl, dist/*.tar.gz'
                    }
                }
                stage ('Windows Build py37') {
                    agent { label 'windows' }
                    steps {
                        deleteDir()
                        checkout scm
                        powershell './win.bat 37'
                        powershell './win.bat 37 64'
                        stash name: 'build-artifacts-win-py37', includes: 'dist/*.whl'
                        archiveArtifacts artifacts: 'dist/*.whl, dist/*.tar.gz'
                    }
                }
                stage ('Windows Build py38') {
                    agent { label 'windows' }
                    steps {
                        deleteDir()
                        checkout scm
                        powershell './win.bat 38'
                        powershell './win.bat 38 64'
                        stash name: 'build-artifacts-win-py38', includes: 'dist/*.whl'
                        archiveArtifacts artifacts: 'dist/*.whl, dist/*.tar.gz'
                    }
                }
                stage ('Windows Build py39') {
                    agent { label 'windows' }
                    steps {
                        deleteDir()
                        checkout scm
                        powershell './win.bat 39'
                        powershell './win.bat 39 64'
                        stash name: 'build-artifacts-win-py39', includes: 'dist/*.whl'
                        archiveArtifacts artifacts: 'dist/*.whl, dist/*.tar.gz'
                    }
                }
            }
        }
        stage ('Upload package') {
            agent { docker 'python:3.6' }
            environment {
                PYTHONUSERBASE = '/tmp/python/'
                XDG_CACHE_HOME = '/tmp/.cache/pip'
                TWINE_REPOSITORY_URL = "${env.PYTHON_REPOSITORY_URL}"
            }
            when { branch "master" }
            steps {
                deleteDir()
                unstash 'build-artifacts-linux'
                unstash 'build-artifacts-win-py36'
                unstash 'build-artifacts-win-py37'
                unstash 'build-artifacts-win-py38'
                unstash 'build-artifacts-win-py39'
                sh 'pip install --user twine'
                withCredentials([usernamePassword(credentialsId: "python-repository-login",
                                    usernameVariable: "TWINE_USERNAME",
                                    passwordVariable: "TWINE_PASSWORD")]) {
                    sh "python -m twine upload dist/*"
                }
            }
        }
    }
}
