// The slack channel to notify about non-dry run or non-disabled autorolls
def slackChannel = '#opapp-freely-build'

pipeline {
    agent any
    // Skip regular checkout to allow it to be done in a subdirectory without
    // breaking Jenkinsfile support (https://stackoverflow.com/a/40392423 )
    options { skipDefaultCheckout() }
    parameters {
        // BEWARE! Jenkins parameter changes are not picked up until AFTER the
        // build with the change is done! See
        // https://stackoverflow.com/a/46813693 for details
        string(
            name: 'DEPS_REPO',
            defaultValue: 'git@github.com:OpenRedButtonProject/chromium.git',
            description: 'Git URL to repo containing DEPS file to update'
        )
        string(
            name: 'DEPS_REPO_BRANCH',
            defaultValue: 'main',
            description: 'Branch to update of repo containing DEPS file'
        )
        booleanParam(
            name: 'DISABLE_ROLL',
            defaultValue: false,
            description: 'If true, don\'t try to perform a roll'
        )
        booleanParam(
            name: 'ROLL_DRY_RUN',
            defaultValue: false,
            description: 'If true, perform the roll locally but don\'t push the commit'
        )
    }
    stages {
        stage('checkout') {
            when {
                beforeAgent true
                not { expression { return params.DISABLE_ROLL } }
            }
            steps {
                echo 'Checking out ORB...'
                dir('orb') {
                    script {
                        def scmVars = checkout scm
                        env.ROLL_COMMIT = scmVars.GIT_COMMIT
                    }
                }
                echo 'Checking out depot_tools...'
                dir('depot_tools') {
                    checkout scmGit(
                        branches: [[name: '0aa5b44d94fce8569c0ee851bc02cc29110f5dc6']],
                        userRemoteConfigs: [
                            [
                                url: 'https://chromium.googlesource.com/chromium/tools/depot_tools.git'
                            ]
                        ]
                    )
                }
                echo 'Checking out Chromium...'
                dir('chromium') {
                    checkout scmGit(
                        branches: [[name: DEPS_REPO_BRANCH]],
                        extensions: [
                            sparseCheckout(sparseCheckoutPaths: [
                                [path: '/DEPS'], [path: 'build/orb_scripts/roll_orb.py']
                            ]),
                            cloneOption(shallow: true)
                        ],
                        userRemoteConfigs: [
                            [
                                credentialsId: 'obs-gitbot-ssh',
                                url: params.DEPS_REPO
                            ]
                        ]
                    )
                }
            }
        }
        stage('roll') {
            steps {
                sh '''
if [ "${DISABLE_ROLL}" = "true" ]; then
    echo "ORB auto rolling is disabled by DISABLE_ROLL parameter"
    exit 0
fi
echo "Rolling ORB dependency forward in Chromium..."
export PATH="${WORKSPACE}/depot_tools:${PATH}"
export DEPOT_TOOLS_UPDATE=0
export EMAIL="gitbot@oceanbluesoftware.co.uk"
export GIT_AUTHOR_NAME="Jenkins Autoroller"
export GIT_COMMITTER_NAME="OBS Git Bot"
python3 chromium/build/orb_scripts/roll_orb.py "${WORKSPACE}/orb" ${ROLL_COMMIT}
'''
            }
        }
        stage('push') {
            when {
                beforeAgent true
                allOf {
                    not { expression { return params.DISABLE_ROLL } }
                    not { expression { return params.ROLL_DRY_RUN } }
                }
            }
            steps {
                // NOTE: the running user's ~/.ssh/known_hosts must already
                // have the GitHub hosts within it otherwise the Git push will
                // fail the authenticity check. This can be done by
                // running
                // [ -d ~/.ssh ] || mkdir ~/.ssh && chmod 700 ~/.ssh
                // ssh-keyscan -H github.com >> ~/.ssh/known_hosts
                // chmod 600 ~/.ssh/known_hosts
                // as the user the agent runs as (by default "jenkins") if this
                // job is running directly on the host.
                sshagent(credentials: ['obs-gitbot-ssh']) {
                    dir('chromium') {
                        sh '''
git branch -f ${DEPS_REPO_BRANCH}
git push origin ${DEPS_REPO_BRANCH}
'''
                    }
                }
            }
        }
    }
    post {
        always {
            script {
                if (!(params.ROLL_DRY_RUN || params.DISABLE_ROLL)) {
                    slackSend channel: slackChannel, color: buildResultColor(), message: buildResultSlackMessage()
                }
            }
        }
    }
}

String buildResultColor() {
    def resultToColor = [FAILURE: '#FF0000', SUCCESS: '#00FF00', UNSTABLE: '#FFFF00', ABORTED: '#999999']
    return resultToColor[currentBuild.result]
}

String buildOutcome() {
    def resultToOutcome = [FAILURE: 'failed', SUCCESS: 'succeeded', UNSTABLE: 'finished with failing tests', ABORTED: 'was aborted']
    return resultToOutcome[currentBuild.result]
}

String buildResultSlackMessage() {
    return "Build <${BUILD_URL}|${JOB_NAME} #${BUILD_NUMBER}> " + buildOutcome()
}
