import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


def print_metrics(y_true, y_pred):
    """
    Prints classification metrics.

    Args:
        y_true (pd.Series): True labels.
        y_pred (pd.Series): Predicted labels.
    """
    logging.info("Calculating metrics...")
    report = classification_report(y_true, y_pred)
    conf_matrix = confusion_matrix(y_true, y_pred)
    logging.info(f"Classification Report:\n{report}")
    logging.info(f"Confusion Matrix:\n{conf_matrix}")
    logging.info("Metrics calculated.")
